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

#ifdef __cplusplus
extern "C" {
#endif

// #define LED_PROFILE(_lc_name, _uc_name)
#define LED_PROFILES \
	LED_PROFILE(normal, NORMAL) \
	LED_PROFILE(hdr,    HDR)

enum led_profile_id {
#define LED_PROFILE(_lc_name, _uc_name) \
	LED_PROFILE_ ## _uc_name,
LED_PROFILES
#undef LED_PROFILE
	NUM_LED_PROFILES,
};

#ifdef __cplusplus
} // extern "C"

#ifndef NO_QSTR
# include <Arduino.h>

# include <array>
# include <bitset>
# include <mutex>

# include "led_profile.h"
#endif

namespace aurcor {

class LEDProfiles {
public:
	LEDProfiles(const __FlashStringHelper *bus_name_);
	~LEDProfiles() = default;

	LEDProfile& get(enum led_profile_id id);
	LEDProfile::Result load(enum led_profile_id id);
	LEDProfile::Result save(enum led_profile_id id);

private:
	static const std::array<const char *,NUM_LED_PROFILES> names_;

	LEDProfile::Result auto_load(enum led_profile_id id, bool reload);

	const __FlashStringHelper *bus_name_;
	std::mutex mutex_;
	std::array<LEDProfile,NUM_LED_PROFILES> profiles_;
	std::bitset<NUM_LED_PROFILES> loaded_{};
};

} // namespace aurcor
#endif
