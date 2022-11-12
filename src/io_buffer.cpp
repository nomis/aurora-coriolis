/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022  Simon Arlott
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

#include "aurcor/io_buffer.h"

#include <Arduino.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>

namespace aurcor {

IOBuffer::IOBuffer(size_t size) : buf_(size) {
}

size_t IOBuffer::read_available() {
	std::lock_guard lock{mutex_};
	return read_available_locked();
}

int IOBuffer::read(bool wait) {
	std::unique_lock lock{mutex_};
	size_t available;

	while (1) {
		available = read_available_locked();

		if (available == 0) {
			if (wait) {
				cv_read_.wait(lock);
				if (stop_)
					return -1;
				continue;
			}
		}

		break;
	}

	if (available > 0) {
		uint8_t c = pop();
		take_locked(1);
		return c;
	} else {
		return -1;
	}
}

int IOBuffer::read_available(const uint8_t *&buf, bool wait) {
	std::unique_lock lock{mutex_};
	size_t available;

	while (1) {
		available = read_available_locked();

		if (available == 0) {
			if (wait) {
				cv_read_.wait(lock);
				if (stop_)
					return -1;
				continue;
			}
		}

		lock.unlock();
		break;
	}

	buf = &buf_[read_];
	return available;
}

void IOBuffer::read_consume(size_t count) {
	if (buf_.size() - read_ > count) {
		read_ += count;
	} else {
		read_ = count - (buf_.size() - read_);
	}

	std::lock_guard lock{mutex_};
	take_locked(count);
}

size_t IOBuffer::write_available() {
	std::lock_guard lock{mutex_};
	return write_available_locked();
}

void IOBuffer::write(int c) {
	if (c != -1) {
		push(c);
		std::lock_guard lock{mutex_};
		give_locked(1);
	}
}

size_t IOBuffer::write(const uint8_t *buf, size_t count, bool wait) {
	std::unique_lock lock{mutex_};
	size_t available;

	while (1) {
		available = write_available_locked();

		if (available == 0) {
			if (wait) {
				cv_write_.wait(lock);
				if (stop_)
					return 0;
			} else {
				return 0;
			}
		} else {
			lock.unlock();
			break;
		}
	}

	if (available > count)
		available = count;

	std::memcpy(&buf_[write_], buf, available);
	if (buf_.size() - write_ > available) {
		write_ += available;
	} else {
		write_ = available - (buf_.size() - write_);
	}
	lock.lock();
	give_locked(available);
	return available;
}

void IOBuffer::stop() {
	std::lock_guard lock{mutex_};
	stop_ = true;
	cv_read_.notify_all();
	cv_write_.notify_all();
}

size_t IOBuffer::read_available_locked() {
	if (buf_.size() - read_ >= used_) {
		return used_;
	} else {
		return buf_.size() - read_;
	}
}

uint8_t IOBuffer::pop() {
	uint8_t c = buf_[read_];

	if (read_ == buf_.size() - 1) {
		read_ = 0;
	} else {
		read_++;
	}

	return c;
}

void IOBuffer::take_locked(size_t count) {
	if (used_ == buf_.size())
		cv_write_.notify_all();
	used_ -= count;
}

size_t IOBuffer::write_available_locked() {
	if (buf_.size() - write_ >= buf_.size() - used_) {
		return buf_.size() - used_;
	} else {
		return buf_.size() - write_;
	}
}

void IOBuffer::push(uint8_t c) {
	buf_[write_] = c;

	if (write_ == buf_.size() - 1) {
		write_ = 0;
	} else {
		write_++;
	}
}

void IOBuffer::give_locked(size_t count) {
	if (used_ == 0)
		cv_read_.notify_all();
	used_ += count;
}

} // namespace aurcor
