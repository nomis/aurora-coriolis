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

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace aurcor {

class IOBuffer {
public:
	IOBuffer(size_t size);

	size_t read_available();
	int read(bool wait);

	int read_available(const uint8_t *&buf, bool wait);
	void read_consume(size_t count);

	size_t write_available();
	void write(int c);
	size_t write(const uint8_t *buf, size_t count, bool wait);

	void stop();

private:
	size_t read_available_locked();
	uint8_t pop();
	void take_locked(size_t count);

	size_t write_available_locked();
	void push(uint8_t c);
	void give_locked(size_t count);

	std::vector<uint8_t> buf_;
	std::mutex mutex_;
	std::condition_variable cv_read_;
	std::condition_variable cv_write_;
	size_t read_{0};
	size_t write_{0};
	size_t used_{0};
	bool stop_{false};
};

} // namespace aurcor
