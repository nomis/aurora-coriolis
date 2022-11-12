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

#include <algorithm>
#include <cstdint>

extern "C" {
    #include <py/mpconfig.h>
}

namespace aurcor {

static inline uint8_t int_to_u8(int value) {
    return std::max(0, std::min(UINT8_MAX, value));
}

static inline mp_float_t float_0to1(mp_float_t value) {
    return std::max((mp_float_t)0.0, std::min((mp_float_t)1.0, value));
}

} // namespace aurcor
