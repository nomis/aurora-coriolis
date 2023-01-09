/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2023  Simon Arlott
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
#include <unordered_set>
#include <utility>

#include "led_profiles.h"

namespace aurcor {

struct Refresh {
	std::unordered_set<std::string> buses;
	std::unordered_set<std::string> presets;
	std::unordered_set<std::pair<std::string,enum led_profile_id>,BusLEDProfileHash> profiles;
	std::unordered_set<std::string> scripts;
};

} // namespace aurcor
