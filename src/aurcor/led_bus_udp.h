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

#pragma once

#include <Arduino.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

extern "C" {
	#include <py/obj.h>
}

#include <uuid/log.h>

#include "memory_pool.h"

namespace aurcor {

class LEDBus;

class LEDBusUDP {
public:
	static constexpr size_t MAX_PACKET_LEN = 1472;
	static constexpr unsigned int DEFAULT_QUEUE_SIZE = 3;
	static constexpr unsigned int MIN_QUEUE_SIZE = 1;
	static constexpr unsigned int MAX_QUEUE_SIZE = 50;

	static void setup(size_t bus_count);

	LEDBusUDP(const LEDBus &bus);
	~LEDBusUDP();

	void loop();
	void start();
	void receive(bool wait, mp_obj_t packets);
	void interrupt();
	void stop();

private:
	static uuid::log::Logger logger_;
	static std::shared_ptr<MemoryPool> buffers_;

	void listen();
	void receive();
	void close();

	const LEDBus &bus_;
	uint16_t port_{0};
	int fd_{-1};

	std::mutex mutex_;
	std::condition_variable cv_;
	bool running_{false};
	std::unique_ptr<MemoryBlock> next_packet_;
	std::deque<std::unique_ptr<MemoryBlock>> packets_;
};

struct UDPPacket {
	uint64_t receive_time_us;
	struct sockaddr_in source_address;
	uint8_t data[LEDBusUDP::MAX_PACKET_LEN];
	size_t length;
};

} // namespace aurcor
