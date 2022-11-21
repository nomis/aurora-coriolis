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

#include <unity.h>

#include <array>
#include <string>

#include "test_led_bus.h"
#include "test_micropython.h"

// Use float values that are out of the normal range

static std::string fn = R"python(
def fn(n):
	return list([(aurcor.HUE_RANGE // 8 * x / aurcor.HUE_RANGE + 1.0,
		aurcor.MAX_VALUE * (10 - x) // 10 / aurcor.MAX_VALUE) for x in range(1, 8)])[0:n]
)python";

#include "../common_hv.ipp"
