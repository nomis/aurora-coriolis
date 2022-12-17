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

#include "aurcor/preset.h"

#include <mutex>
#include <shared_mutex>
#include <string>

#include <uuid/log.h>

#include "aurcor/app.h"
#include "aurcor/micropython.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "preset";

namespace aurcor {

uuid::log::Logger Preset::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

Preset::Preset(App &app, std::shared_ptr<LEDBus> bus, std::string name)
		: app_(app), bus_(bus), name_(name) {

}

std::string Preset::name() const {
	std::shared_lock data_lock{data_mutex_};

	if (name_.empty()) {
		return "<unnamed>";
	} else {
		return name_;
	}
}

void Preset::name(const std::string &name) {
	std::unique_lock data_lock{data_mutex_};

	if (name_ != name) {
		name_ = name;
		modified_ = true;
	}
}

std::string Preset::script() const {
	std::shared_lock data_lock{data_mutex_};
	return script_;
}

void Preset::script(const std::string &script) {
	std::unique_lock data_lock{data_mutex_};

	if (script_ != script) {
		script_ = script;
		modified_ = true;
		if (running_)
			script_changed_ = true;
	}
}

std::shared_ptr<std::shared_ptr<Preset>> Preset::edit() {
	auto ref = editing_.lock();

	if (!ref) {
		ref = std::make_shared<std::shared_ptr<Preset>>(shared_from_this());
		editing_ = ref;
	}

	return ref;
}

void Preset::loop() {
	if (running_ && !mp_->running()) {
		stop_time_ms_ = uuid::get_uptime_ms();
		running_ = false;
	}

	if (!restart())
		return;

	if (!app_.detach(bus_))
		return;

	if (script_changed_) {
		script_changed_ = false;

		logger_.trace(F("Change script on %s[%s] to \"%s\""),
			bus_->type(), bus_->name(), script_.c_str());
	} else {
		logger_.trace(F("Run script \"%s\" on %s[%s]"),
			script_.c_str(), bus_->type(), bus_->name());
	}

	mp_ = std::make_shared<MicroPythonFile>(script_, bus_, shared_from_this());
	app_.attach(bus_, mp_);

	if (mp_->start()) {
		running_ = true;
	} else {
		running_ = false;
		app_.detach(bus_, mp_);
		mp_.reset();
		stop_time_ms_ = uuid::get_uptime_ms();
	}
}

bool Preset::restart() const {
	if (script_changed_)
		return true;

	if (running_)
		return false;

	if (!stop_time_ms_)
		return true;

	return uuid::get_uptime_ms() - stop_time_ms_ >= RESTART_TIME_MS;
}

} // namespace aurcor
