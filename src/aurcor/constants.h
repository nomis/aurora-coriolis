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

#ifndef AURCOR_MAX_LEDS
# define AURCOR_MAX_LEDS 1000
#endif

#ifndef AURCOR_MIN_RATIO_LEDS
# define AURCOR_MIN_RATIO_LEDS 50
#endif

#ifdef __cplusplus
# include <cstddef>

namespace aurcor {

static constexpr size_t MAX_LEDS = AURCOR_MAX_LEDS;
static constexpr size_t MIN_RATIO_LEDS = AURCOR_MIN_RATIO_LEDS;

} // namespace aurcor

#endif
