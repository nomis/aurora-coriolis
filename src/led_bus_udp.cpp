/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2024  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NO_QSTR
# include "aurcor/led_bus_udp.h"

# include <Arduino.h>

# include <arpa/inet.h>
# include <errno.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>

# include <array>
# include <cstring>
# include <memory>
# include <mutex>

# include <uuid/log.h>

extern "C" {
	# include <py/obj.h>
}

# include "aurcor/led_bus.h"
# include "aurcor/memory_pool.h"
# include "aurcor/micropython.h"
#endif

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

using uuid::log::Level;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-bus-udp";

namespace aurcor {

uuid::log::Logger LEDBusUDP::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

std::shared_ptr<MemoryPool> LEDBusUDP::buffers_ = std::make_shared<MemoryPool>(
	sizeof(UDPPacket), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

LEDBusUDP::LEDBusUDP(const LEDBus &bus) : bus_(bus) {

}

LEDBusUDP::~LEDBusUDP() {
	close();
}

void LEDBusUDP::setup(size_t bus_count) {
	buffers_->resize(bus_count * (MAX_QUEUE_SIZE + 1));
}

void LEDBusUDP::close() {
	if (fd_ != -1) {
		::close(fd_);
		fd_ = -1;
	}
}

void LEDBusUDP::loop() {
	std::lock_guard lock{mutex_};
	listen();
	receive();
}

void LEDBusUDP::listen() {
	uint16_t port = running_ ? bus_.udp_port() : 0;

	if (port_ != port) {
		if (fd_ != -1) {
			close();
			logger_.trace("Stopped listening on port %u for %s[%s]", port_, bus_.type(), bus_.name());
		}

		if (port) {
			fd_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (fd_ == -1) {
				logger_.trace("socket(IPPROTO_UDP): %d", errno);
			}

			if (fd_ != -1) {
				int one = 1;

				if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
					logger_.trace("setsockopt(SO_REUSEADDR): %d", errno);
					close();
				}
			}

			if (fd_ != -1) {
				int flags = fcntl(fd_, F_GETFL);
				if (flags == -1) {
					logger_.trace("fcntl(F_GETFL): %d", errno);
					close();
				}

				if (fd_ != -1) {
					if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
						logger_.trace("fcntl(F_SETFL): %d", errno);
						close();
					}
				}
			}

			if (fd_ != -1) {
				struct sockaddr_in addr{};

				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(port);

				if (bind(fd_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr))) {
					logger_.trace("bind(%u): %d", port, errno);
					close();
				}
			}

			if (fd_ != -1) {
				logger_.trace("Started listening on port %u for %s[%s]", port, bus_.type(), bus_.name());
			}
		}

		port_ = port;
	}
}

void LEDBusUDP::receive() {
	if (fd_ == -1) {
		return;
	}

	if (!next_packet_) {
		next_packet_ = buffers_->allocate();
		if (!next_packet_) {
			logger_.crit("Out of memory receiving for %s[%s]", bus_.type(), bus_.name());
			close();
			return;
		}
	}

	UDPPacket *packet = reinterpret_cast<UDPPacket*>(next_packet_->begin());
	socklen_t addrlen = sizeof(packet->source_address);

	packet->receive_time_us = esp_timer_get_time();

	ssize_t len = recvfrom(fd_, packet->data, sizeof(packet->data), 0,
		reinterpret_cast<struct sockaddr *>(&packet->source_address), &addrlen);
	if (len == -1) {
		return;
	}

	packet->length = len;

	while (packets_.size() >= bus_.udp_queue_size()) {
		packets_.pop_front();
	}

	packets_.push_back(std::move(next_packet_));
	cv_.notify_all();
}

void LEDBusUDP::start() {
	std::lock_guard lock{mutex_};
	running_ = true;
}

void LEDBusUDP::receive(bool wait, mp_obj_t packets) {
	micropython_nlr_begin();

	std::unique_lock lock{mutex_};

	micropython_nlr_try();

	if (running_) {
		if (wait && packets_.empty()) {
			mp_handle_pending(true);
			MP_THREAD_GIL_EXIT();
			cv_.wait(lock);
			MP_THREAD_GIL_ENTER();

			if (packets_.empty()) {
				mp_handle_pending(true);
			}
		}

		while (!packets_.empty()) {
			static const std::array<qstr,3> fields{
				MP_QSTR_receive_ticks64_us,
				MP_QSTR_source_address,
				MP_QSTR_data,
			};

			UDPPacket *packet = reinterpret_cast<UDPPacket*>(packets_.front()->begin());

			char source_address[INET_ADDRSTRLEN] = { 0 };

			if (::inet_ntop(packet->source_address.sin_family,
					&packet->source_address.sin_addr, source_address,
					sizeof(source_address)) == nullptr) {
				snprintf(source_address, sizeof(source_address), "%u",
					ntohl(packet->source_address.sin_addr.s_addr));
			}

			std::array tuple{
				mp_obj_new_str(source_address, strlen(source_address)),
				mp_obj_new_int_from_uint(ntohs(packet->source_address.sin_port)),
			};

			std::array<mp_obj_t,fields.size()> items{
				mp_obj_new_int_from_ll(packet->receive_time_us),
				mp_obj_new_tuple(tuple.size(), tuple.begin()),
				mp_obj_new_bytes(packet->data, packet->length),
			};

			mp_obj_list_append(packets,
				mp_obj_new_attrtuple(fields.begin(), fields.size(), items.begin()));

			packets_.pop_front();
		}
	}

	micropython_nlr_finally();
	micropython_nlr_end();
}

void LEDBusUDP::interrupt() {
	std::lock_guard lock{mutex_};
	cv_.notify_all();
}

void LEDBusUDP::stop() {
	std::lock_guard lock{mutex_};
	running_ = false;
	packets_.clear();
	cv_.notify_all();
}

} // namespace aurcor
