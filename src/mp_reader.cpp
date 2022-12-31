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

#include "aurcor/mp_reader.h"

extern "C" {
	#include <py/mperrno.h>
	#include <py/reader.h>
	#include <py/runtime.h>
}

#include <shared_mutex>

#include "app/fs.h"
#include "aurcor/app.h"

namespace aurcor {

namespace micropython {

Reader::Reader(const char *filename)
	: lock_(App::file_mutex()), file_(app::FS.open(filename)) {
}

mp_reader_t Reader::from_file(const char *filename) {
	MP_THREAD_GIL_EXIT();
	auto *reader = new Reader{filename};
	MP_THREAD_GIL_ENTER();

	if (!reader->file_) {
		delete reader;
		mp_raise_OSError(MP_ENOENT);
	}

	return {reader, &Reader::readbyte, &Reader::close};
}

mp_uint_t Reader::readbyte(void *data) {
	int c = reinterpret_cast<Reader*>(data)->file_.read();
	return c == -1 ? MP_READER_EOF : c;
}

void Reader::close(void *data) {
	delete reinterpret_cast<Reader*>(data);
}

} // namespace micropython

} // namespace aurcor
