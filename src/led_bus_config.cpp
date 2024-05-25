/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2024  Simon Arlott
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
#include <limits>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/log.h>

#include "app/fs.h"
#include "app/util.h"
#include "aurcor/app.h"
#include "aurcor/led_bus_format.h"
#include "aurcor/led_bus_udp.h"
#include "aurcor/modaurcor.h"
#include "aurcor/preset.h"
#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

namespace cbor = qindesign::cbor;
using app::FS;
using uuid::log::Level;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-bus-config";

namespace aurcor {

uuid::log::Logger LEDBusConfig::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

LEDBusConfig::LEDBusConfig(const char *bus_name, size_t default_length)
		: bus_name_(bus_name), default_length_(default_length) {
	length_constrained(default_length_);
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
	length_ = uint_constrain(value, MAX_LEDS, MIN_LEDS);
}

LEDBusFormat LEDBusConfig::format() const {
	std::shared_lock data_lock{data_mutex_};
	return format_;
}

void LEDBusConfig::format(LEDBusFormat value) {
	std::unique_lock data_lock{data_mutex_};
	if (format_ != value || !format_set_) {
		format_ = value;
		format_set_ = true;
		data_lock.unlock();
		save();
	}
}

unsigned int LEDBusConfig::reset_time_us() const {
	std::shared_lock data_lock{data_mutex_};
	return reset_time_us_;
}

void LEDBusConfig::reset_time_us(unsigned int value) {
	std::unique_lock data_lock{data_mutex_};
	if (reset_time_us_ != value || !reset_time_us_set_) {
		reset_time_us_constrained(value);
		reset_time_us_set_ = true;
		data_lock.unlock();
		save();
	}
}

void LEDBusConfig::reset_time_us_constrained(unsigned int value) {
	reset_time_us_ = uint_constrain(value, std::numeric_limits<typeof(reset_time_us_)>::max());
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

void LEDBusConfig::default_preset(std::string_view value) {
	std::unique_lock data_lock{data_mutex_};
	if (default_preset_ != value) {
		default_preset_ = value;
		data_lock.unlock();
		save();
	}
}

unsigned int LEDBusConfig::default_fps() const {
	std::shared_lock data_lock{data_mutex_};
	return default_fps_;
}

void LEDBusConfig::default_fps(unsigned int value) {
	std::unique_lock data_lock{data_mutex_};
	if (default_fps_ != value || !default_fps_set_) {
		default_fps_constrained(value);
		default_fps_set_ = true;
		data_lock.unlock();
		save();
	}
}

void LEDBusConfig::default_fps_constrained(unsigned int value) {
	default_fps_ = uint_constrain(value, micropython::PyModule::MAX_FPS);
}

uint16_t LEDBusConfig::udp_port() const {
	std::shared_lock data_lock{data_mutex_};
	return udp_port_;
}

void LEDBusConfig::udp_port(uint16_t value) {
	std::unique_lock data_lock{data_mutex_};
	if (udp_port_ != value || !udp_port_set_) {
		udp_port_ = value;
		udp_port_set_ = true;
		data_lock.unlock();
		save();
	}
}

unsigned int LEDBusConfig::udp_queue_size() const {
	std::shared_lock data_lock{data_mutex_};
	return udp_queue_size_;
}

void LEDBusConfig::udp_queue_size(unsigned int value) {
	std::unique_lock data_lock{data_mutex_};
	if (udp_queue_size_ != value || !udp_queue_size_set_) {
		udp_queue_size_constrained(value);
		udp_queue_size_set_ = true;
		data_lock.unlock();
		save();
	}
}

void LEDBusConfig::udp_queue_size_constrained(unsigned int value) {
	udp_queue_size_ = uint_constrain(value, LEDBusUDP::MAX_QUEUE_SIZE, LEDBusUDP::MIN_QUEUE_SIZE);
}

void LEDBusConfig::reset() {
	std::unique_lock data_lock{data_mutex_};

	reset_locked();
	data_lock.unlock();
	save();
}

void LEDBusConfig::reset_locked() {
	length_constrained(default_length_);
	length_set_ = false;
	format_ = DEFAULT_FORMAT;
	format_set_ = false;
	reset_time_us_ = DEFAULT_RESET_TIME_US;
	reset_time_us_set_ = false;
	reverse_ = false;
	default_preset_ = "";
	default_fps_ = DEFAULT_DEFAULT_FPS;
	default_fps_set_ = false;
	udp_port_ = 0;
	udp_port_set_ = false;
	udp_queue_size_ = LEDBusUDP::DEFAULT_QUEUE_SIZE;
	udp_queue_size_set_ = false;
}

std::string LEDBusConfig::make_filename(const char *bus_name) {
	std::string filename;

	filename.append(DIRECTORY_NAME);
	filename.append("/");
	filename.append(bus_name);
	filename.append(FILENAME_EXT);

	return filename;
}

bool LEDBusConfig::load() {
	auto filename = make_filename(bus_name_);
	std::unique_lock data_lock{data_mutex_};
	std::shared_lock file_lock{App::file_mutex()};

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
		} else if (key == "format") {
			std::string value;

			if (!app::read_text(reader, value))
				return false;

			if (!LEDBusFormats::uc_id(value, format_))
				return false;
		} else if (key == "reset_time_us") {
			uint64_t value;

			if (!cbor::expectUnsignedInt(reader, &value))
				return false;

			reset_time_us_constrained(value);
			reset_time_us_set_ = true;
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
		} else if (key == "default_fps") {
			uint64_t value;

			if (!cbor::expectUnsignedInt(reader, &value))
				return false;

			default_fps_constrained(value);
			default_fps_set_ = true;
		} else if (key == "udp_port") {
			uint64_t value;

			if (!cbor::expectUnsignedInt(reader, &value))
				return false;

			if (value <= std::numeric_limits<typeof(udp_port_)>::max()) {
				udp_port_ = value;
				udp_port_set_ = true;
			}
		} else if (key == "udp_queue_size") {
			uint64_t value;

			if (!cbor::expectUnsignedInt(reader, &value))
				return false;

			udp_queue_size_constrained(value);
			udp_queue_size_set_ = true;
		} else if (!reader.isWellFormed()) {
			return false;
		}
	}

	return result;
}

bool LEDBusConfig::save() {
	auto filename = make_filename(bus_name_);
	std::shared_lock data_lock{data_mutex_};
	std::unique_lock file_lock{App::file_mutex()};

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
	size_t values = 0;

	if (length_set_)
		values++;
	values++; // format is always saved
	if (reset_time_us_set_)
		values++;
	if (reverse_)
		values++;
	if (!default_preset_.empty())
		values++;
	if (default_fps_set_)
		values++;
	if (udp_port_set_)
		values++;
	if (udp_queue_size_set_)
		values++;

	writer.beginMap(values);

	if (length_set_) {
		app::write_text(writer, "length");
		writer.writeUnsignedInt(length_);
	}

	app::write_text(writer, "format");
	app::write_text(writer, LEDBusFormats::uc_name(format_));

	if (reset_time_us_set_) {
		app::write_text(writer, "reset_time_us");
		writer.writeUnsignedInt(reset_time_us_);
	}

	if (reverse_) {
		app::write_text(writer, "reverse");
		writer.writeBoolean(true);
	}

	if (!default_preset_.empty()) {
		app::write_text(writer, "default_preset");
		app::write_text(writer, default_preset_);
	}

	if (default_fps_set_) {
		app::write_text(writer, "default_fps");
		writer.writeUnsignedInt(default_fps_);
	}

	if (udp_port_set_) {
		app::write_text(writer, "udp_port");
		writer.writeUnsignedInt(udp_port_);
	}

	if (udp_queue_size_set_) {
		app::write_text(writer, "udp_queue_size");
		writer.writeUnsignedInt(udp_queue_size_);
	}
}

} // namespace aurcor
