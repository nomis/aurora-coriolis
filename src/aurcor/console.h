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

#include "app/console.h"

#include <memory>
#include <string>
#include <vector>

#include "led_bus.h"
#include "led_profiles.h"

namespace aurcor {

class LEDProfile;
class Preset;

class AurcorShell: public app::AppShell {
public:
	~AurcorShell() override = default;

	void enter_bus_context(std::shared_ptr<LEDBus> bus);
	void enter_bus_profile_context(enum led_profile_id profile);
	void enter_bus_profile_context(std::shared_ptr<LEDBus> bus, enum led_profile_id profile);
	void enter_bus_preset_context(std::shared_ptr<std::shared_ptr<Preset>> preset);
	void enter_bus_preset_context(std::shared_ptr<LEDBus> bus, std::shared_ptr<std::shared_ptr<Preset>> preset);
	void enter_bus_preset_cfglist_context(const std::string &name);
	void enter_bus_preset_cfgset_context(const std::string &name);
	bool exit_context() override;

	inline std::shared_ptr<LEDBus>& bus() { return bus_; }
	inline LEDProfile& profile() { return bus_->profile(profile_); }
	inline enum led_profile_id profile_id() { return profile_; }
	inline Preset& preset() { return **preset_; }
	inline std::string& preset_cfg_name() { return preset_cfg_name_; }

	void set_command(Shell &shell) override;
	bool preset_active(bool exit = true);

protected:
	AurcorShell(app::App &app, Stream &stream, unsigned int context, unsigned int flags);

	void display_banner() override;
	std::string context_text() override;

private:
	std::shared_ptr<LEDBus> bus_;
	enum led_profile_id profile_{LED_PROFILE_NORMAL};
	std::shared_ptr<std::shared_ptr<Preset>> preset_;
	std::string preset_cfg_name_;
};

} // namespace aurcor
