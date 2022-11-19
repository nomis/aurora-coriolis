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

#include <Arduino.h>
#include <ArduinoJson.h>

#include <limits>
#include <map>
#include <mutex>
#include <shared_mutex>

#include <uuid/console.h>
#include <uuid/log.h>

#include "app/json.h"
#include "constants.h"

namespace aurcor {

class LEDProfiles;

class LEDProfile {
	friend LEDProfiles;
public:
	using index_t = uint16_t;
	static_assert(std::numeric_limits<index_t>::max() > MAX_LEDS,
		"Index type is too small to cover all LEDs");

	static_assert(std::numeric_limits<index_t>::min() == 0, "Index type must be unsigned");
	static_assert(std::numeric_limits<index_t>::max() <= std::numeric_limits<unsigned int>::max(),
		"Index type is too large to be stored in an unsigned int");
	static constexpr unsigned int MAX_INDEX = std::numeric_limits<index_t>::max();

	// These are in priority order to allow combining errors
	enum Result : uint8_t {
		OK = 0,
		FULL,
		NOT_FOUND,
		OUT_OF_RANGE,
		PARSE_ERROR,
		IO_ERROR,
	};

	LEDProfile() = default;
	~LEDProfile() = default;

	void print(uuid::console::Shell &shell, size_t limit = MAX_LEDS) const;
	void transform(uint8_t *data, size_t size) const;

	std::vector<unsigned int> indexes() const;
	Result get(unsigned int index, uint8_t &r, uint8_t &g, uint8_t &b) const;
	Result set(unsigned int index, int r, int g, int b);
	Result adjust(unsigned int index, int r, int g, int b);
	Result move(unsigned int src, unsigned int dst);
	Result copy(unsigned int src, unsigned int dst);
	Result remove(unsigned int index);
	void clear();
	bool compact(size_t limit = SIZE_MAX);

	/* Not protected by a mutex; assumes modifications only happen from one
	 * thread. Making this thread-safe would require an extra mutex to avoid
	 * blocking other readers when saving the profile.
	 */
	bool modified() const { return modified_; }

protected:
	Result load(const char *bus_name, const char *profile_name, bool automatic);
	Result save(const char *bus_name, const char *profile_name);

private:
	static constexpr size_t MAX_RATIOS = (MAX_LEDS + MIN_RATIO_LEDS - 1) / MIN_RATIO_LEDS;
	static_assert(MAX_RATIOS > 0, "Must allow at least one ratio configuration entry");
	static constexpr size_t BUFFER_SIZE = JSON_ARRAY_SIZE(MAX_RATIOS)
		+ MAX_RATIOS * (JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(3));
#ifdef ENV_NATIVE
	static constexpr bool VERBOSE = true;
#else
	static constexpr bool VERBOSE = false;
#endif

	struct Ratio {
		union {
			struct {
				uint8_t r;
				uint8_t g;
				uint8_t b;
			} __attribute__((packed));
			uint8_t v[3];
		} __attribute__((packed));

		friend inline bool operator==(const Ratio &lhs, const Ratio &rhs) {
			return lhs.r == rhs.r
				&& lhs.g == rhs.g
				&& lhs.b == rhs.b;
		}

		friend inline bool operator!=(const Ratio &lhs, const Ratio &rhs) {
			return !(lhs == rhs);
		}
	} __attribute__((packed));

	/*
	 * Default to very dim LEDs to avoid overloading the power supply if the
	 * profile is scaled to limit power use and there's an error loading it.
	 */
	static constexpr Ratio DEFAULT_RATIO{8, 8, 8};

	static bool valid_index(unsigned int index) {
		return index <= MAX_INDEX
			&& (index_t)index < MAX_LEDS;
	}

	static bool valid_value(int value) {
		return value >= 0
			&& value <= UINT8_MAX;
	}

	static Result downgrade_result(Result &current, Result result) {
		current = std::max(current, result);
		return result;
	}

	Result add(index_t index, const Ratio &ratio);
	Result add_default();
	Result remove(const std::map<index_t,Ratio>::iterator &it);
	Ratio get(index_t index) const;
	Result copy(unsigned int src, unsigned int dst, bool move);

	static std::string make_filename(const char *bus_name, const char *profile_name);

	Result load(app::JsonDocument &doc);
	Result load_ratio_configs(ArduinoJson::JsonArray &array);
	Result load_ratio_config(ArduinoJson::JsonArray &array);
	static Result get_ratio_config_index(ArduinoJson::JsonArray &array,
		ArduinoJson::JsonArray::iterator &it, index_t &index);
	static Result get_ratio_config_ratio(ArduinoJson::JsonArray &array,
		ArduinoJson::JsonArray::iterator &it, Ratio &ratio);
	static Result get_ratio_config_ratio_value(
		ArduinoJson::JsonVariant &element, uint8_t &ratio_value);

	void save(app::JsonDocument &doc, index_t index, const Ratio &ratio);

	static uuid::log::Logger logger_;

	mutable std::shared_mutex data_mutex_;
	std::map<index_t,Ratio> ratios_;

	bool modified_{false};
};

} // namespace aurcor
