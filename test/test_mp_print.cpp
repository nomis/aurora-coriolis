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

#include "test_mp_print.h"

extern "C" {
	#include <py/mphal.h>
}

#include <uuid/log.h>

#include "aurcor/mp_print.h"

TestPrint::TestPrint(uuid::log::Level level)
		: level_(reinterpret_cast<const char *>(uuid::log::format_level_uppercase(level))) {
	level_.append(":");
}

void TestPrint::begin_line() {
	::mp_hal_stdout_tx_strn(level_.c_str(), level_.length());
}

void TestPrint::print_part_line(const char *str, size_t len) {
	::mp_hal_stdout_tx_strn(str, len);
}

void TestPrint::end_line() {
	::mp_hal_stdout_tx_strn("\r\n", 2);
}

TestPrint::~TestPrint() {
	if (line_started())
		end_line();
}
