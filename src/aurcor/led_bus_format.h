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

#include <array>
#include <string>
#include <vector>

namespace aurcor {

// #define LED_BUS_FORMAT(_uc_name, _r_idx, _g_idx, _b_idx)
#define LED_BUS_FORMATS \
	LED_BUS_FORMAT(RGB, 0, 1, 2) \
	LED_BUS_FORMAT(RBG, 0, 2, 1) \
	LED_BUS_FORMAT(GRB, 1, 0, 2) \
	LED_BUS_FORMAT(GBR, 1, 2, 0) \
	LED_BUS_FORMAT(BRG, 2, 0, 1) \
	LED_BUS_FORMAT(BGR, 2, 1, 0)

enum class LEDBusFormat {
#define LED_BUS_FORMAT(_uc_name, _r_idx, _g_idx, _b_idx) \
	_uc_name,

LED_BUS_FORMATS
#undef LED_BUS_FORMAT
};

class LEDBusFormats {
public:
	static std::vector<std::string> uc_names();
	static const char* uc_name(LEDBusFormat format);
	static bool uc_id(const std::string &name, LEDBusFormat &format);

private:
#define LED_BUS_FORMAT(_uc_name, _r_idx, _g_idx, _b_idx) +1
	static constexpr size_t NUM_LED_BUS_FORMATS = LED_BUS_FORMATS;
#undef LED_BUS_FORMAT
	static const std::array<const char *,NUM_LED_BUS_FORMATS> uc_names_;

	LEDBusFormats() = delete;
	~LEDBusFormats() = delete;
};

} // namespace aurcor
