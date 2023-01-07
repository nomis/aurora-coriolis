/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2023  Simon Arlott
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

#include <limits>
#include <map>
#include <mutex>
#include <shared_mutex>

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/console.h>
#include <uuid/log.h>

#include "constants.h"

namespace aurcor {

class LEDBusConfig {
public:
	static constexpr uint16_t DEFAULT_RESET_TIME_US = LED_BUS_RESET_TIME_US;
	static constexpr uint16_t DEFAULT_DEFAULT_FPS = 50;

	static constexpr const char *DIRECTORY_NAME = "/buses";
	static constexpr const char *FILENAME_EXT = ".cbor";

	LEDBusConfig(const char *bus_name, size_t default_length);
	~LEDBusConfig() = default;

	size_t length() const;
	void length(size_t value);

	unsigned int reset_time_us() const;
	void reset_time_us(unsigned int value);

	bool reverse() const;
	void reverse(bool value);

	std::string default_preset() const;
	void default_preset(std::string value);

	unsigned int default_fps() const;
	void default_fps(unsigned int value);

	void reset();
	inline void reload() { load(); }

protected:
	void length_constrained(size_t value);
	void reset_time_us_constrained(unsigned int value);
	void default_fps_constrained(unsigned int value);

	bool load();
	bool save();

private:
	static std::string make_filename(const char *bus_name);

	void reset_locked();

	bool load(qindesign::cbor::Reader &reader);
	void save(qindesign::cbor::Writer &writer);

	static uuid::log::Logger logger_;

	const char *bus_name_;
	mutable std::shared_mutex data_mutex_;
	size_t default_length_;
	size_t length_;
	std::string default_preset_;
	uint16_t reset_time_us_{DEFAULT_RESET_TIME_US};
	uint16_t default_fps_{DEFAULT_DEFAULT_FPS};
	bool length_set_{false};
	bool reset_time_us_set_{false};
	bool default_fps_set_{false};
	bool reverse_{false};
};

} // namespace aurcor
