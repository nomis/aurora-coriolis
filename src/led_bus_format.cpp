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

#include "aurcor/led_bus_format.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "aurcor/led_bus_format.h"

namespace aurcor {

namespace ledbusformats {

#define LED_BUS_FORMAT(_uc_name, _r_idx, _g_idx, _b_idx) \
	static const char *uc_name_ ## _uc_name = #_uc_name;

LED_BUS_FORMATS
#undef LED_BUS_FORMAT


} // namespace ledbusformats

const std::array<const char *,LEDBusFormats::NUM_LED_BUS_FORMATS> LEDBusFormats::uc_names_ = {
#define LED_BUS_FORMAT(_uc_name, _r_idx, _g_idx, _b_idx) \
	ledbusformats::uc_name_ ## _uc_name,

LED_BUS_FORMATS
#undef LED_BUS_FORMAT
};

std::vector<std::string> LEDBusFormats::uc_names() {
	return {uc_names_.begin(), uc_names_.end()};
}

const char* LEDBusFormats::uc_name(LEDBusFormat format) {
	size_t value = (size_t)format;
	return uc_names_[value];
}

bool LEDBusFormats::uc_id(const std::string &name, LEDBusFormat &format) {
	for (size_t value = 0; value < NUM_LED_BUS_FORMATS; value++) {
		if (uc_names_[value] == name) {
			format = static_cast<LEDBusFormat>(value);
			return true;
		}
	}

	return false;
}

} // namespace aurcor
