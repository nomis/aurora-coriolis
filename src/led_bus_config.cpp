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

#include "aurcor/led_bus_config.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/log.h>

#include "app/fs.h"
#include "app/util.h"
#include "aurcor/preset.h"
#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

namespace cbor = qindesign::cbor;
using app::FS;
using uuid::log::Level;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-bus-config";

static const char *directory_name = "/buses";

namespace aurcor {

uuid::log::Logger LEDBusConfig::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

LEDBusConfig::LEDBusConfig(const char *bus_name, size_t default_length)
		: bus_name_(bus_name) {
	length_constrained(default_length);
	load();
}

size_t LEDBusConfig::length() const {
	std::shared_lock data_lock{data_mutex_};
	return length_;
}

void LEDBusConfig::length(size_t value) {
	std::unique_lock data_lock{data_mutex_};
	if (length_ != value || !length_set_) {
		length_constrained(value);
		length_set_ = true;
		data_lock.unlock();
		save();
	}
}

void LEDBusConfig::length_constrained(size_t value) {
	length_ = std::max(MIN_LEDS, std::min(MAX_LEDS, value));
}

bool LEDBusConfig::reverse() const {
	std::shared_lock data_lock{data_mutex_};
	return reverse_;
}

void LEDBusConfig::reverse(bool value) {
	std::unique_lock data_lock{data_mutex_};
	if (reverse_ != value) {
		reverse_ = value;
		data_lock.unlock();
		save();
	}
}

std::string LEDBusConfig::default_preset() const {
	std::shared_lock data_lock{data_mutex_};
	return default_preset_;
}

void LEDBusConfig::default_preset(std::string value) {
	std::unique_lock data_lock{data_mutex_};
	if (default_preset_ != value) {
		default_preset_ = value;
		data_lock.unlock();
		save();
	}
}

void LEDBusConfig::reset() {
	std::unique_lock data_lock{data_mutex_};

	reset_locked();
	data_lock.unlock();
	save();
}

void LEDBusConfig::reset_locked() {
	length_constrained(0);
	length_set_ = true;
	reverse_ = false;
	default_preset_ = "";
}

std::string LEDBusConfig::make_filename(const char *bus_name) {
	std::string filename;

	filename.append(directory_name);
	filename.append("/");
	filename.append(bus_name);
	filename.append(".cbor");

	return filename;
}

bool LEDBusConfig::load() {
	auto filename = make_filename(bus_name_);
	std::unique_lock data_lock{data_mutex_};

	logger_.debug(F("Reading config from file %s"), filename.c_str());

	auto file = FS.open(filename.c_str(), "r");
	if (file) {
		cbor::Reader reader{file};

		if (!cbor::expectValue(reader, cbor::DataType::kTag, cbor::kSelfDescribeTag))
			file.seek(0);

		auto result = load(reader);

		if (!result) {
			logger_.err(F("Config file %s contains invalid data that has been ignored"), filename.c_str());
		}

		return result;
	} else {
		logger_.debug(F("Unable to open config file %s for reading"), filename.c_str());
		return false;
	}
}

bool inline LEDBusConfig::load(cbor::Reader &reader) {
	bool result = true;
	uint64_t length;
	bool indefinite;

	if (!cbor::expectMap(reader, &length, &indefinite) || indefinite) {
		logger_.trace(F("File does not contain a definite length map"));
		return false;
	}

	reset_locked();

	while (length-- > 0) {
		std::string key;

		if (!app::read_text(reader, key))
			return false;

		if (key == "length") {
			uint64_t value;

			if (!cbor::expectUnsignedInt(reader, &value))
				return false;

			length_constrained(value);
			length_set_ = true;
		} else if (key == "reverse") {
			if (!cbor::expectBoolean(reader, &reverse_))
				return false;
		} else if (key == "default_preset") {
			std::string value;

			if (!app::read_text(reader, value))
				return false;

			if (value.length() > Preset::MAX_NAME_LENGTH)
				value.resize(Preset::MAX_NAME_LENGTH);

			default_preset_ = value;
		} else if (!reader.isWellFormed()) {
			return false;
		}
	}

	return result;
}

bool LEDBusConfig::save() {
	auto filename = make_filename(bus_name_);
	std::shared_lock data_lock{data_mutex_};

	logger_.notice(F("Writing config to file %s"), filename.c_str());

	auto file = FS.open(filename.c_str(), "w", true);
	if (!file) {
		logger_.err(F("Unable to open config file %s for writing"), filename.c_str());
		return false;
	}

	cbor::Writer writer{file};

	writer.writeTag(cbor::kSelfDescribeTag);
	save(writer);

	if (file.getWriteError()) {
		logger_.err(F("Failed to write config file %s: %u"), filename.c_str(), file.getWriteError());
		file.close();
		FS.remove(filename.c_str());
		return false;
	} else {
		return true;
	}
}

void LEDBusConfig::save(cbor::Writer &writer) {
	writer.beginMap(3);

	app::write_text(writer, "length");
	writer.writeUnsignedInt(length_);

	app::write_text(writer, "reverse");
	writer.writeBoolean(reverse_);

	app::write_text(writer, "default_preset");
	app::write_text(writer, default_preset_);
}

} // namespace aurcor