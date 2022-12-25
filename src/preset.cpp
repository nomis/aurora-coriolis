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

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/log.h>

#include "app/fs.h"
#include "app/util.h"
#include "aurcor/app.h"
#include "aurcor/micropython.h"
#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

namespace cbor = qindesign::cbor;
using app::FS;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "preset";

static const char *directory_name = "/presets";

namespace aurcor {

uuid::log::Logger Preset::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

Preset::Preset(App &app, std::shared_ptr<LEDBus> bus, std::string name)
		: app_(app), bus_(bus), name_(name) {
	reset();
}

std::vector<std::string> Preset::names() {
	return list_filenames(directory_name, ".cbor");
}

std::string Preset::name(bool allow_unnamed) const {
	std::shared_lock data_lock{data_mutex_};

	if (name_.empty() && !allow_unnamed) {
		return "<unnamed>";
	} else {
		return name_;
	}
}

bool Preset::name(std::string name) {
	if (!allowed_file_name(name))
		return false;

	if (name.length() > MAX_NAME_LENGTH)
		name.resize(MAX_NAME_LENGTH);

	std::unique_lock data_lock{data_mutex_};

	if (name_ != name) {
		name_ = name;
		modified_ = true;
	}

	return true;
}

std::string Preset::description() const {
	std::shared_lock data_lock{data_mutex_};
	return description_;
}

bool Preset::description(std::string description) {
	if (!description_constrained(description))
		return false;

	std::unique_lock data_lock{data_mutex_};

	if (description_ != description) {
		description_ = description;
		modified_ = true;
	}

	return true;
}

bool Preset::description_constrained(std::string &description) {
	if (!allowed_text(description))
		return false;

	if (description.length() > MAX_DESCRIPTION_LENGTH)
		description.resize(MAX_DESCRIPTION_LENGTH);

	return true;
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
		if (running_) {
			script_changed_ = true;
		} else {
			stop_time_ms_ = 0;
		}
	}
}

bool Preset::reverse() const {
	std::shared_lock data_lock{data_mutex_};
	return reverse_;
}

void Preset::reverse(bool reverse) {
	std::unique_lock data_lock{data_mutex_};

	if (reverse_ != reverse) {
		reverse_ = reverse;
		modified_ = true;
	}
}

void Preset::register_config(mp_obj_t dict) {
	micropython_nlr_begin();

	std::unique_lock data_lock{data_mutex_};

	micropython_nlr_try();

	config_.register_config(dict);
	config_changed_ = true;

	micropython_nlr_finally();
	micropython_nlr_end();
}

bool Preset::populate_config(mp_obj_t dict) {
	volatile bool ret = false;

	micropython_nlr_begin();

	std::shared_lock data_lock{data_mutex_};

	micropython_nlr_try();

	if (config_changed_) {
		config_.populate_config(dict);
		config_changed_ = false;
		ret = true;
	}

	micropython_nlr_finally();
	micropython_nlr_end();

	return ret;
}

std::shared_ptr<std::shared_ptr<Preset>> Preset::edit() {
	auto ref = editing_.lock();

	if (!ref) {
		ref = std::make_shared<std::shared_ptr<Preset>>(shared_from_this());
		editing_ = ref;
	}

	return ref;
}

std::string Preset::make_filename() const {
	std::string filename;

	filename.append(directory_name);
	filename.append("/");
	filename.append(name_);
	filename.append(".cbor");

	return filename;
}

void Preset::reset() {
	if (!script_.empty()) {
		if (running_) {
			script_changed_ = true;
		} else {
			stop_time_ms_ = 0;
		}
	}

	description_ = "";
	script_ = "";
	reverse_ = false;
	modified_ = false;
}

Preset::Result Preset::load() {
	auto filename = make_filename();
	std::unique_lock data_lock{data_mutex_};

	if (bus_)
		logger_.info(F("Reading preset from file %s to bus %s"), filename.c_str(), bus_->name());

	auto file = FS.open(filename.c_str(), "r");
	if (file) {
		cbor::Reader reader{file};

		if (!cbor::expectValue(reader, cbor::DataType::kTag, cbor::kSelfDescribeTag))
			file.seek(0);

		auto result = load(reader);

		switch (result) {
		case Result::FULL:
			logger_.err(F("Preset file %s contains too many config values (truncated)"), filename.c_str());
			break;

		case Result::NOT_FOUND:
		case Result::PARSE_ERROR:
		case Result::IO_ERROR:
			logger_.err(F("Preset file %s contains invalid data that has been ignored"), filename.c_str());
			break;

		case Result::OK:
		default:
			break;
		}

		return result;
	} else {
		logger_.err(F("Unable to open preset file %s for reading"), filename.c_str());
		return Result::NOT_FOUND;
	}
}

Preset::Result Preset::load(cbor::Reader &reader) {
	uint64_t entries;
	bool indefinite;

	if (!cbor::expectMap(reader, &entries, &indefinite) || indefinite) {
		logger_.trace(F("File does not contain a definite length map"));
		return Result::PARSE_ERROR;
	}

	auto old_script = script_;
	reset();
	modified_ = true;

	while (entries-- > 0) {
		std::string key;

		if (!app::read_text(reader, key))
			return Result::PARSE_ERROR;

		if (key == "desc") {
			std::string value;

			if (!app::read_text(reader, value))
				return Result::PARSE_ERROR;

			if (description_constrained(value))
				description_ = value;
		} else if (key == "script") {
			std::string value;

			if (!app::read_text(reader, value))
				return Result::PARSE_ERROR;

			script_ = value;
		} else if (key == "reverse") {
			if (!cbor::expectBoolean(reader, &reverse_))
				return Result::PARSE_ERROR;
		} else if (key == "config") {
			if (!reader.isWellFormed())
				return Result::PARSE_ERROR;
		} else if (!reader.isWellFormed()) {
			return Result::PARSE_ERROR;
		}
	}

	if (script_ == old_script)
		script_changed_ = false;

	modified_ = false;
	config_changed_ = true;
	return Result::OK;
}

Preset::Result Preset::save() {
	std::shared_lock data_lock{data_mutex_};
	if (name_.empty())
		return Result::NOT_FOUND;
	auto filename = make_filename();

	logger_.notice(F("Writing preset from bus %s to file %s"), bus_->name(), filename.c_str());

	auto file = FS.open(filename.c_str(), "w", true);
	if (!file) {
		logger_.err(F("Unable to open preset file %s for writing"), filename.c_str());
		return Result::IO_ERROR;
	}

	cbor::Writer writer{file};

	writer.writeTag(cbor::kSelfDescribeTag);
	save(writer);

	if (file.getWriteError()) {
		logger_.err(F("Failed to write preset file %s: %u"), filename.c_str(), file.getWriteError());
		file.close();
		FS.remove(filename.c_str());
		return Result::IO_ERROR;
	} else {
		modified_ = false;
		return Result::OK;
	}
}

void Preset::save(cbor::Writer &writer) {
	writer.beginMap(4);

	app::write_text(writer, "desc");
	app::write_text(writer, description_);

	app::write_text(writer, "script");
	app::write_text(writer, script_);

	app::write_text(writer, "reverse");
	writer.writeBoolean(reverse_);

	app::write_text(writer, "config");
	config_.save(writer);
}

Preset::Result Preset::rename(const Preset &destination) {
	std::unique_lock data_lock{data_mutex_};
	std::shared_lock data_lock2{destination.data_mutex_};

	if (name_.empty() || destination.name_.empty())
		return Result::NOT_FOUND;

	auto filename_from = make_filename();
	auto filename_to = destination.make_filename();

	if (FS.exists(filename_from.c_str())) {
		if (FS.exists(filename_to.c_str())) {
			logger_.notice(F("Deleting preset file %s"), filename_to.c_str());
			FS.remove(filename_to.c_str());
		}

		logger_.notice(F("Renaming preset file from %s to %s"), filename_from.c_str(), filename_to.c_str());
		if (FS.rename(filename_from.c_str(), filename_to.c_str())) {
			name_ = destination.name_;
			return Result::OK;
		} else {
			return Result::IO_ERROR;
		}
	} else {
		return Result::NOT_FOUND;
	}
}

Preset::Result Preset::remove() {
	std::shared_lock data_lock{data_mutex_};

	if (name_.empty())
		return Result::NOT_FOUND;

	auto filename = make_filename();

	if (FS.exists(filename.c_str())) {
		logger_.notice(F("Deleting preset file %s"), filename.c_str());
		if (FS.remove(filename.c_str())) {
			modified_ = true;
			return Result::OK;
		} else {
			return Result::IO_ERROR;
		}
	} else {
		return Result::NOT_FOUND;
	}
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
