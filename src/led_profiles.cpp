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

#include "aurcor/led_profiles.h"

#include <Arduino.h>

#include <array>
#include <mutex>

#include "aurcor/led_bus.h"
#include "aurcor/led_profile.h"

namespace aurcor {

namespace ledprofiles {

#define LED_PROFILE(_lc_name, _uc_name) \
	static const char *lc_name_ ## _lc_name = #_lc_name;
LED_PROFILES
#undef LED_PROFILE

} // namespace ledprofiles

const std::array<const char *,NUM_LED_PROFILES> LEDProfiles::names_ = {
#define LED_PROFILE(_lc_name, _uc_name) \
	ledprofiles::lc_name_ ## _lc_name,
LED_PROFILES
#undef LED_PROFILE
};

LEDProfiles::LEDProfiles(const char *bus_name) : bus_name_(bus_name) {

}

LEDProfile& LEDProfiles::get(enum led_profile_id id) {
	size_t profile = (size_t)id;

	auto_load(id, false);
	return profiles_[profile];
}

LEDProfile::Result LEDProfiles::load(enum led_profile_id id) {
	return auto_load(id, true);
}

LEDProfile::Result LEDProfiles::save(enum led_profile_id id) {
	size_t profile = (size_t)id;

	auto_load(id, false);
	return profiles_[profile].save(bus_name_, names_[profile]);
}

LEDProfile::Result LEDProfiles::auto_load(enum led_profile_id id, bool reload) {
	size_t profile = (size_t)id;
	std::unique_lock lock{mutex_};

	if (!loaded_[profile]) {
		loaded_[profile] = true;
	} else if (!reload) {
		return LEDProfile::Result::OK;
	} else {
		lock.unlock();
	}

	return profiles_[profile].load(bus_name_, names_[profile], !reload);
}

} // namespace aurcor