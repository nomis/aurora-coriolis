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
	LEDBusConfig(const char *bus_name, size_t default_length);
	~LEDBusConfig() = default;

	size_t length() const;
	void length(size_t value);

	bool reverse() const;
	void reverse(bool value);

	void reset();

protected:
	void length_constrained(size_t value);

	bool load();
	bool save();

private:
#ifdef ENV_NATIVE
	static constexpr bool VERBOSE = true;
#else
	static constexpr bool VERBOSE = false;
#endif

	static std::string make_filename(const char *bus_name);

	bool load(qindesign::cbor::Reader &reader);
	void save(qindesign::cbor::Writer &writer);

	static uuid::log::Logger logger_;

	const char *bus_name_;
	mutable std::shared_mutex data_mutex_;
	size_t length_;
	bool length_set_{false};
	bool reverse_{false};
};

} // namespace aurcor
