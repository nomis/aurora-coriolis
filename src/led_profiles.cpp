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
#include <vector>

#include "aurcor/led_bus.h"
#include "aurcor/led_profile.h"

namespace aurcor {

namespace ledprofiles {

#define LED_PROFILE(_lc_name, _uc_name) \
	static const char *lc_name_ ## _lc_name = #_lc_name;

LED_PROFILES
#undef LED_PROFILE

} // namespace ledprofiles

const std::array<const char *,NUM_LED_PROFILES> LEDProfiles::lc_names_ = {
#define LED_PROFILE(_lc_name, _uc_name) \
	ledprofiles::lc_name_ ## _lc_name,

LED_PROFILES
#undef LED_PROFILE
};

LEDProfiles::LEDProfiles(const char *bus_name) : bus_name_(bus_name) {

}

std::vector<std::string> LEDProfiles::lc_names() {
	return {lc_names_.begin(), lc_names_.end()};
}

const char* LEDProfiles::lc_name(enum led_profile_id id) {
	size_t profile = (size_t)id;
	return lc_names_[profile];
}

bool LEDProfiles::lc_id(const std::string &name, enum led_profile_id &id) {
	for (size_t profile = MIN_ID; profile <= MAX_ID; profile++) {
		if (lc_names_[profile] == name) {
			id = static_cast<enum led_profile_id>(profile);
			return true;
		}
	}

	return false;
}

LEDProfile& LEDProfiles::get(enum led_profile_id id) {
	size_t profile = (size_t)id;

	auto_load(id, false);
	return profiles_[profile];
}

Result LEDProfiles::load(enum led_profile_id id) {
	return auto_load(id, true);
}

bool LEDProfiles::loaded(enum led_profile_id id) {
	size_t profile = (size_t)id;
	std::unique_lock lock{mutex_};

	return loaded_[profile];
}

Result LEDProfiles::save(enum led_profile_id id) {
	size_t profile = (size_t)id;

	auto_load(id, false);
	return profiles_[profile].save(bus_name_, lc_names_[profile]);
}

Result LEDProfiles::auto_load(enum led_profile_id id, bool reload) {
	size_t profile = (size_t)id;
	std::unique_lock lock{mutex_};

	if (!loaded_[profile]) {
		loaded_[profile] = true;
	} else if (!reload) {
		return Result::OK;
	} else {
		lock.unlock();
	}

	return profiles_[profile].load(bus_name_, lc_names_[profile], !reload);
}

} // namespace aurcor
