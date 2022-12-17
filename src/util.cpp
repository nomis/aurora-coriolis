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

#include "aurcor/util.h"

#include <string>

namespace aurcor {

bool allowed_file_name(const std::string &name) {
	for (auto c : name) {
		if (c >= 'a' && c <= 'z')
			continue;
		else if (c >= 'A' && c <= 'Z')
			continue;
		else if (c >= '0' && c <= '9')
			continue;
		else if (c >= '+' && c <= '.') /* +,-. */
			continue;
		else if (c == '_')
			continue;
		return false;
	}

	return !name.empty();
}

} // namespace aurcor
