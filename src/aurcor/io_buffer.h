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

#pragma once

#include <Arduino.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <thread>

namespace aurcor {

template<typename T, size_t N>
class IOBuffer {
	static_assert(N > 0);
	static_assert(N <= std::numeric_limits<T>::max());
	static_assert(std::numeric_limits<T>::min() == 0);

public:
	using len_t = T;

	inline T read_available() {
		std::lock_guard lock{mutex_};
		return read_available_locked();
	}

	inline int read(bool wait) {
		std::unique_lock lock{mutex_};
		T available;

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

	inline int read(const uint8_t *&buf, bool wait) {
		std::unique_lock lock{mutex_};
		T available;

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

	inline void take(T count) {
		std::lock_guard lock{mutex_};
		take_locked(count);
	}

	inline T write_available() {
		std::lock_guard lock{mutex_};
		return write_available_locked();
	}

	inline void write(int c) {
		if (c != -1) {
			push(c);
			give(1);
		}
	}

	inline size_t write(const uint8_t *buf, size_t count, bool wait) {
		std::unique_lock lock{mutex_};
		T available;

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
		lock.lock();
		give_locked(available);
		return available;
	}

	void stop() {
		std::lock_guard lock{mutex_};
		stop_ = true;
		cv_read_.notify_all();
		cv_write_.notify_all();
	}

private:
	inline T read_available_locked() {
		if (N - read_ >= used_) {
			return used_;
		} else {
			return N - read_;
		}
	}

	inline uint8_t pop() {
		uint8_t c = buf_[read_];

		if (read_ == N - 1) {
			read_ = 0;
		} else {
			read_++;
		}

		return c;
	}

	inline void take_locked(T count) {
		if (N - read_ > count) {
			read_ += count;
		} else {
			read_ = count - (N - read_);
		}

		if (used_ == N)
			cv_write_.notify_all();
		used_ -= count;
	}

	inline T write_available_locked() {
		if (N - write_ >= N - used_) {
			return N - used_;
		} else {
			return N - write_;
		}
	}

	inline void push(uint8_t c) {
		buf_[write_] = c;

		if (write_ == N - 1) {
			write_ = 0;
		} else {
			write_++;
		}
	}

	inline void give(T count) {
		std::lock_guard lock{mutex_};
		give_locked(count);
	}

	inline void give_locked(T count) {
		if (N - write_ > count) {
			write_ += count;
		} else {
			write_ = count - (N - write_);
		}

		if (used_ == 0)
			cv_read_.notify_all();
		used_ += count;
	}

	std::array<uint8_t,N> buf_;
	std::mutex mutex_;
	std::condition_variable cv_read_;
	std::condition_variable cv_write_;
	T read_ = 0;
	T write_ = 0;
	T used_ = 0;
	bool stop_ = false;
};

} // namespace aurcor
