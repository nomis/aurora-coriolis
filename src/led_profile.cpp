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

#include "aurcor/led_profile.h"

#include <Arduino.h>
#include <ArduinoJson.hpp>

#include <mutex>
#include <shared_mutex>

#include <uuid/console.h>
#include <uuid/log.h>

#include "app/fs.h"
#include "app/json.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

using app::FS;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-profile";

static const char __pstr__print_header1[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM =
	"LEDs         Red Green Blue";
static const char __pstr__print_header2[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM =
	"------------ --- ----- ----";
static const char __pstr__print_row[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM =
	"%5u..%-5u" " %3u  %3u  %3u";

static const char __pstr__directory_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "/profiles";

namespace aurcor {

uuid::log::Logger LEDProfile::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

void LEDProfile::print(uuid::console::Shell &shell, size_t limit) const {
	std::shared_lock data_lock{data_mutex_};
	index_t begin = 0;
	index_t index = 0;
	Ratio ratio = DEFAULT_RATIO;

	if (limit > 0) {
		shell.printfln(FPSTR(__pstr__print_header1));
		shell.printfln(FPSTR(__pstr__print_header2));
	}

	for (auto it = ratios_.cbegin(); limit-- > 0; index++) {
		if (it != ratios_.cend()) {
			if (it->index == index) {
				if (index > 0) {
					shell.printfln(FPSTR(__pstr__print_row), begin, index - 1,
						ratio.r, ratio.g, ratio.b);

					begin = index;
				}
				ratio = it->ratio;
				++it;
			}
		} else {
			shell.printfln(FPSTR(__pstr__print_row), begin, index + limit,
					ratio.r, ratio.g, ratio.b);
			break;
		}
	}
}

void LEDProfile::transform(char *data, size_t size) const {
	std::shared_lock data_lock{data_mutex_};
	index_t index = 0;
	Ratio ratio = DEFAULT_RATIO;

	for (auto it = ratios_.cbegin(); size > 0; index++) {
		if (it != ratios_.cend() && it->index == index) {
			ratio = it->ratio;
			++it;
		}

		for (uint8_t i = 0; i < sizeof(ratio.v) && size > 0; i++) {
			*data = *data * ratio.v[i] / UINT8_MAX;
			data++;
			size--;
		}
	}
}

LEDProfile::Result LEDProfile::add(const RatioConfig &ratio) {
	if (!ratio.is_default()) {
		size_t size = ratios_.size();

		if (size > MAX_RATIOS
				|| (size == MAX_RATIOS && !compact(1))) {
			return Result::FULL;
		}

		ratios_.insert(ratio);
		modified_ = true;
	}

	return Result::OK;
}

LEDProfile::Result LEDProfile::remove(const std::set<RatioConfig>::iterator &it) {
	if (it != ratios_.end()) {
		ratios_.erase(it);
		modified_ = true;
		return Result::OK;
	} else {
		return Result::NOT_FOUND;
	}
}

std::vector<int> LEDProfile::indexes() const {
	std::vector<int> values{{0}};

	for (auto& ratio : ratios_)
		values.push_back(ratio.index);

	return values;
}

LEDProfile::Result LEDProfile::get(int index, uint8_t &r, uint8_t &g, uint8_t &b) const {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::shared_lock data_lock{data_mutex_};
	Ratio ratio = get((index_t)index);

	r = ratio.r;
	g = ratio.g;
	b = ratio.b;

	return Result::OK;
}

LEDProfile::Result LEDProfile::set(int index, int r, int g, int b) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	r = std::min(std::max(0, r), UINT8_MAX);
	g = std::min(std::max(0, g), UINT8_MAX);
	b = std::min(std::max(0, b), UINT8_MAX);

	std::unique_lock data_lock{data_mutex_};
	RatioConfig ratio{(index_t)index, {(uint8_t)r, (uint8_t)g, (uint8_t)b}};

	remove(ratios_.find(ratio));
	return add(ratio);
}

LEDProfile::Result LEDProfile::adjust(int index, int r, int g, int b) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};
	RatioConfig cfg{(index_t)index, get((index_t)index)};

	cfg.ratio.r = std::min(std::max(0, (int)cfg.ratio.r + r), UINT8_MAX);
	cfg.ratio.g = std::min(std::max(0, (int)cfg.ratio.g + g), UINT8_MAX);
	cfg.ratio.b = std::min(std::max(0, (int)cfg.ratio.b + b), UINT8_MAX);

	return add(cfg);
}

LEDProfile::Ratio LEDProfile::get(index_t index) const {
	RatioConfig cfg{(index_t)index, DEFAULT_RATIO};
	auto it = ratios_.find(cfg);

	if (it != ratios_.end()) {
		return it->ratio;
	} else {
		for (auto& ratio : ratios_) {
			if (ratio.index < index) {
				cfg.ratio = ratio.ratio;
			} else {
				break;
			}
		}

		return cfg.ratio;
	}
}

LEDProfile::Result LEDProfile::move(int src, int dst) {
	return copy(src, dst, true);
}

LEDProfile::Result LEDProfile::copy(int src, int dst) {
	return copy(src, dst, false);
}

LEDProfile::Result LEDProfile::copy(int src, int dst, bool move) {
	if (!valid_index(src) || !valid_index(dst))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};
	RatioConfig dst_ratio{(index_t)dst, DEFAULT_RATIO};
	auto src_it = ratios_.find(RatioConfig{(index_t)src, DEFAULT_RATIO});

	if (src_it != ratios_.end()) {
		dst_ratio.ratio = src_it->ratio;

		if (move)
			remove(src_it);
	} else if (src != 0) {
		// The implied DEFAULT_RATIO entry at index 0 can always be copied
		return Result::NOT_FOUND;
	}

	remove(ratios_.find(dst_ratio));
	return add(dst_ratio);
}

LEDProfile::Result LEDProfile::remove(int index) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};

	return remove(ratios_.find(RatioConfig{(index_t)index, DEFAULT_RATIO}));
}

void LEDProfile::clear() {
	std::unique_lock data_lock{data_mutex_};

	if (!ratios_.empty()) {
		ratios_.clear();
		modified_ = true;
	}
}

bool LEDProfile::compact(size_t limit) {
	std::unique_lock data_lock{data_mutex_};
	size_t removed = 0;

	if (!ratios_.empty() && removed < limit) {
		if (ratios_.size() >= 2) {
			auto next = ratios_.end();
			auto it = ratios_.end();

			do {
				it--;

				if (next != ratios_.end() && next->ratio == it->ratio) {
					remove(next);
					removed++;
				}

				next = it;
			} while (it != ratios_.begin() && removed < limit);
		}

		if (removed < limit) {
			// The loop above can't remove the first element so the set is never empty
			auto first = ratios_.begin();

			if (first->ratio == DEFAULT_RATIO) {
				remove(first);
				removed++;
			}
		}
	}

	return removed > 0;
}

std::string LEDProfile::make_filename(const __FlashStringHelper *bus_name,
		const __FlashStringHelper *profile_name) {
	std::string filename;

	filename.append(uuid::read_flash_string(FPSTR(__pstr__directory_name)));
	filename.append(1, 	'/');
	filename.append(uuid::read_flash_string(bus_name));
	filename.append(1, 	'.');
	filename.append(uuid::read_flash_string(profile_name));
	filename.append(uuid::read_flash_string(F(".json")));

	return filename;
}

LEDProfile::Result LEDProfile::load(const __FlashStringHelper *bus_name,
		const __FlashStringHelper *profile_name) {
	auto filename = make_filename(bus_name, profile_name);
	std::unique_lock data_lock{data_mutex_};

	logger_.notice(F("Reading profile from file %s"), filename.c_str());

	const char mode[2] = {'r', '\0'};
	auto file = FS.open(filename.c_str(), mode);
	if (file) {
		app::JsonDocument doc{BUFFER_SIZE};

		auto error = ArduinoJson::deserializeJson(doc, file);
		if (error) {
			logger_.err(F("Failed to read profile file %s: %s"), filename.c_str(), error.c_str());
			return Result::PARSE_ERROR;
		} else {
			auto result = load(doc);

			switch (result) {
			case Result::FULL:
				logger_.err(F("Profile file %s contains too many entries (truncated)"), filename.c_str());
				break;

			case Result::PARSE_ERROR:
				logger_.err(F("Profile file %s contains invalid data that has been ignored"), filename.c_str());
				break;

			default:
				break;
			}

			return result;
		}
	} else {
		logger_.err(F("Unable to open profile file %s for reading"), filename.c_str());
		return Result::IO_ERROR;
	}
}

LEDProfile::Result inline LEDProfile::load(app::JsonDocument &doc) {
	Result result = Result::PARSE_ERROR;
	auto root = doc.as<JsonArray>();

	if (root) {
		ratios_.clear();
		result = load_ratio_configs(root);
		modified_ = result != Result::OK;
	} else {
		if (VERBOSE)
			logger_.trace(F("Document root is not an array"));
	}

	return result;
}

LEDProfile::Result inline LEDProfile::load_ratio_configs(JsonArray &array) {
	Result result = OK;

	for (JsonVariant element : array) {
		auto entry = element.as<JsonArray>();

		if (!entry) {
			if (VERBOSE)
				logger_.trace(F("Ratio config entry is not an array"));

			downgrade_result(result, Result::PARSE_ERROR);
			continue;
		}

		downgrade_result(result, load_ratio_config(entry));
	}

	return result;
}

LEDProfile::Result inline LEDProfile::load_ratio_config(JsonArray &array) {
	Result result = OK;
	index_t index;
	Ratio ratio;

	auto it = array.begin();

	if (downgrade_result(result,
			get_ratio_config_index(array, it, index)) != Result::OK)
		return result;

	++it;
	if (downgrade_result(result,
			get_ratio_config_ratio(array, it, ratio)) != Result::OK)
		return result;

	downgrade_result(result, add(RatioConfig{index, ratio}));

	++it;
	if (it != array.end()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config entry has additional data"));
		downgrade_result(result, Result::PARSE_ERROR);
	}

	return result;
}

LEDProfile::Result inline LEDProfile::get_ratio_config_index(JsonArray &array,
		JsonArray::iterator &it, index_t &index) {
	if (it == array.end()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config entry is missing the index"));
		return Result::PARSE_ERROR;
	}

	if (!it->is<int>()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config index is not an int"));
		return Result::PARSE_ERROR;
	}

	int value = it->as<int>();

	if (!valid_index(value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config index %d is out of range"), value);
		return Result::PARSE_ERROR;
	}

	index = (index_t)value;
	return Result::OK;
}

LEDProfile::Result inline LEDProfile::get_ratio_config_ratio(JsonArray &array,
		JsonArray::iterator &it, Ratio &ratio) {
	Result result = OK;
	uint8_t i = 0;

	if (it == array.end()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config entry is missing the ratio"));
		return Result::PARSE_ERROR;
	}

	if (!it->is<JsonArray>()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio is not an array"));
		return Result::PARSE_ERROR;
	}

	for (JsonVariant element : it->as<JsonArray>()) {
		if (i >= sizeof(ratio.v)) {
			if (VERBOSE)
				logger_.trace(F("Ratio config ratio has too many values"));
			return Result::PARSE_ERROR;
		}

		if (downgrade_result(result,
				get_ratio_config_ratio_value(element, ratio.v[i])) != Result::OK)
			return result;

		i++;
	}

	if (i == (uint8_t)sizeof(ratio.v)) {
		return Result::OK;
	} else {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio has too few values (%u < %u)"),
				i, (uint8_t)sizeof(ratio.v));
		return Result::PARSE_ERROR;
	}
}

LEDProfile::Result inline LEDProfile::get_ratio_config_ratio_value(
		JsonVariant &element, uint8_t &ratio_value) {
	if (!element.is<int>()) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio value is not an int"));
		return Result::PARSE_ERROR;
	}

	int value = element.as<int>();

	if (!valid_value(value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio value %d is out of range"), value);
		return Result::PARSE_ERROR;
	}

	ratio_value = (uint8_t)value;
	return Result::OK;
}

LEDProfile::Result LEDProfile::save(const __FlashStringHelper *bus_name,
		const __FlashStringHelper *profile_name) {
	auto dirname = uuid::read_flash_string(FPSTR(__pstr__directory_name));
	auto filename = make_filename(bus_name, profile_name);
	std::shared_lock data_lock{data_mutex_};

	logger_.notice(F("Writing profile to file %s"), filename.c_str());

	if (!FS.mkdir(dirname.c_str())) {
		logger_.err(F("Unable to create directory %s"), dirname.c_str());
		return Result::IO_ERROR;
	}

	app::JsonDocument doc{BUFFER_SIZE};

	if (!ratios_.empty() && ratios_.begin()->index != 0)
		save(doc, RatioConfig{0, DEFAULT_RATIO});

	for (auto &ratio : ratios_)
		save(doc, ratio);

	if (doc.overflowed()) {
		logger_.err(F("Out of memory saving profile"));
		return Result::IO_ERROR;
	}

	const char mode[2] = {'w', '\0'};
	auto file = FS.open(filename.c_str(), mode);
	if (file) {
		ArduinoJson::serializeJson(doc, file);

		if (file.getWriteError()) {
			logger_.err(F("Failed to write profile file %s: %u"), filename.c_str(), file.getWriteError());
			return Result::IO_ERROR;
		} else {
			modified_ = false;
			return Result::OK;
		}
	} else {
		logger_.err(F("Unable to open profile file %s for writing"), filename.c_str());
		return Result::IO_ERROR;
	}
}

void LEDProfile::save(app::JsonDocument &doc, const RatioConfig &cfg) {
	auto array = doc.createNestedArray();

	array.add(cfg.index);

	array = array.createNestedArray();

	array.add(cfg.ratio.r);
	array.add(cfg.ratio.g);
	array.add(cfg.ratio.b);
}

} // namespace aurcor
