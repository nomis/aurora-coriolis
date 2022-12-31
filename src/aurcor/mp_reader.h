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

extern "C" {
	#include <py/reader.h>
}

#include <shared_mutex>

#include "app/fs.h"

namespace aurcor {

namespace micropython {

class Reader {
public:
	static mp_reader_t from_file(const char *filename);

private:
	Reader(const char *filename);
	~Reader() = default;

	Reader(Reader&&) = delete;
	Reader(const Reader&) = delete;
	Reader& operator=(Reader&&) = delete;
	Reader& operator=(const Reader&) = delete;

	static mp_uint_t readbyte(void *data);
	static void close(void *data);

	std::shared_lock<std::shared_mutex> lock_;
	fs::File file_;
};

} // namespace micropython

} // namespace aurcor
