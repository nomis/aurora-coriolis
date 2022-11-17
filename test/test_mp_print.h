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

#include <string>

#include <uuid/log.h>

#include "aurcor/mp_print.h"

class TestPrint: public aurcor::micropython::LinePrint {
public:
	TestPrint(uuid::log::Level level);
	~TestPrint() override;

protected:
	void begin_line() override;
	void print_part_line(const char *str, size_t len) override;
	void end_line() override;

private:
	std::string level_;
};
