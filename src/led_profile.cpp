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

#include <mutex>
#include <shared_mutex>
#include <string>

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/console.h>
#include <uuid/log.h>

#include "app/fs.h"
#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

namespace cbor = qindesign::cbor;
using app::FS;
using uuid::log::Level;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-profile";

static const char *print_header1 =	"LEDs         Red Green Blue";
static const char *print_header2 = "------------ --- ----- ----";
static const char *print_row     = "%5u..%-5u" " %3u  %3u  %3u";

static const char *directory_name = "/profiles";

namespace aurcor {

uuid::log::Logger LEDProfile::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

void LEDProfile::print(uuid::console::Shell &shell, size_t limit) const {
	std::shared_lock data_lock{data_mutex_};
	index_t begin = 0;
	index_t index = 0;
	Ratio ratio = DEFAULT_RATIO;

	if (limit > 0) {
		shell.println(print_header1);
		shell.println(print_header2);
	}

	for (auto it = ratios_.cbegin(); limit-- > 0; index++) {
		if (it != ratios_.cend()) {
			if (it->first == index) {
				if (index > 0) {
					shell.printfln(print_row, begin, index - 1,
						ratio.r, ratio.g, ratio.b);

					begin = index;
				}
				ratio = it->second;
				++it;
			}
		} else {
			shell.printfln(print_row, begin, index + limit,
				ratio.r, ratio.g, ratio.b);
			return;
		}
	}

	shell.printfln(print_row, begin, index - 1, ratio.r, ratio.g, ratio.b);
}

void LEDProfile::transform(uint8_t *data, size_t size) const {
	std::shared_lock data_lock{data_mutex_};
	index_t index = 0;
	Ratio ratio = DEFAULT_RATIO;

	for (auto it = ratios_.cbegin(); size > 0; index++) {
		if (it != ratios_.cend() && it->first == index) {
			ratio = it->second;
			++it;
		}

		for (uint8_t i = 0; i < sizeof(ratio.v) && size > 0; i++) {
			*data = *data * ratio.v[i] / UINT8_MAX;
			data++;
			size--;
		}
	}
}

LEDProfile::Result LEDProfile::add(index_t index, const Ratio &ratio) {
	if (index != 0 || ratio != DEFAULT_RATIO) {
		size_t size = ratios_.size();

		if (size > MAX_RATIOS
				|| (size == MAX_RATIOS && !compact_locked(1))) {
			return Result::FULL;
		}

		ratios_.emplace(index, ratio);
		modified_ = true;
	}

	return Result::OK;
}

/*
 * Default to very dim LEDs after a parse error to avoid overloading the power
 * supply if the profile is scaled to limit power use and there's an error
 * loading it.
 */
LEDProfile::Result LEDProfile::add_default() {
	if (!ratios_.empty()) {
		auto it = ratios_.end();

		--it;

		index_t index = it->first;

		if (index < MAX_INDEX) {
			return add(index + 1, DEFAULT_RATIO);
		} else {
			return Result::FULL;
		}
	}

	return Result::OK;
}


LEDProfile::Result LEDProfile::remove(const std::map<index_t,Ratio>::iterator &it) {
	if (it != ratios_.end()) {
		ratios_.erase(it);
		modified_ = true;
		return Result::OK;
	} else {
		return Result::NOT_FOUND;
	}
}

std::vector<unsigned int> LEDProfile::indexes() const {
	std::vector<unsigned int> values;

	if (values.empty() || ratios_.begin()->first > 0)
		values.push_back(0);

	for (auto &entry : ratios_) {
		values.push_back(entry.first);
	}

	return values;
}

LEDProfile::Result LEDProfile::get(unsigned int index, uint8_t &r, uint8_t &g, uint8_t &b) const {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::shared_lock data_lock{data_mutex_};
	Ratio ratio = get((index_t)index);

	r = ratio.r;
	g = ratio.g;
	b = ratio.b;

	return Result::OK;
}

LEDProfile::Result LEDProfile::set(unsigned int index, int r, int g, int b) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};
	Ratio ratio{int_to_u8(r), int_to_u8(g), int_to_u8(b)};

	remove(ratios_.find(index));
	return add(index, ratio);
}

LEDProfile::Result LEDProfile::adjust(unsigned int index, int r, int g, int b) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};
	Ratio ratio{get((index_t)index)};

	ratio.r = std::min(std::max(0, (int)ratio.r + r), UINT8_MAX);
	ratio.g = std::min(std::max(0, (int)ratio.g + g), UINT8_MAX);
	ratio.b = std::min(std::max(0, (int)ratio.b + b), UINT8_MAX);

	remove(ratios_.find(index));
	return add((index_t)index, ratio);
}

LEDProfile::Ratio LEDProfile::get(index_t index) const {
	Ratio ratio{DEFAULT_RATIO};

	for (auto &entry : ratios_) {
		if (entry.first <= index) {
			ratio = entry.second;
		} else {
			break;
		}
	}

	return ratio;
}

LEDProfile::Result LEDProfile::move(unsigned int src, unsigned int dst) {
	return copy(src, dst, true);
}

LEDProfile::Result LEDProfile::copy(unsigned int src, unsigned int dst) {
	return copy(src, dst, false);
}

LEDProfile::Result LEDProfile::copy(unsigned int src, unsigned int dst, bool move) {
	if (!valid_index(src) || !valid_index(dst))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};
	Ratio dst_ratio{DEFAULT_RATIO};
	auto src_it = ratios_.find((index_t)src);

	if (src_it != ratios_.end()) {
		dst_ratio = src_it->second;

		if (move)
			remove(src_it);
	} else if (src != 0) {
		// The implied DEFAULT_RATIO entry at index 0 can always be copied
		return Result::NOT_FOUND;
	}

	remove(ratios_.find((index_t)dst));
	return add((index_t)dst, dst_ratio);
}

LEDProfile::Result LEDProfile::remove(unsigned int index) {
	if (!valid_index(index))
		return Result::OUT_OF_RANGE;

	std::unique_lock data_lock{data_mutex_};

	auto result = remove(ratios_.find((index_t)index));
	if (index == 0) {
		return Result::OK;
	} else {
		return result;
	}
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

	return compact_locked(limit);
}

bool LEDProfile::compact_locked(size_t limit) {
	size_t removed = 0;

	if (!ratios_.empty() && removed < limit) {
		if (ratios_.size() >= 2) {
			auto next = ratios_.end();
			auto it = ratios_.end();

			do {
				--it;

				if (next != ratios_.end() && next->second == it->second) {
					remove(next);
					removed++;
				}

				next = it;
			} while (it != ratios_.begin() && removed < limit);
		}

		if (removed < limit) {
			// The loop above can't remove the first element so the set is never empty
			auto first = ratios_.begin();

			if (first->second == DEFAULT_RATIO) {
				remove(first);
				removed++;
			}
		}
	}

	return removed > 0;
}

std::string LEDProfile::make_filename(const char *bus_name, const char *profile_name) {
	std::string filename;

	filename.append(directory_name);
	filename.append("/");
	filename.append(bus_name);
	filename.append(".");
	filename.append(profile_name);
	filename.append(".cbor");

	return filename;
}

LEDProfile::Result LEDProfile::load(const char *bus_name, const char *profile_name,
		bool automatic) {
	auto filename = make_filename(bus_name, profile_name);
	std::unique_lock data_lock{data_mutex_};

	logger_.log(automatic ? Level::DEBUG : Level::NOTICE,
		F("Reading profile from file %s"), filename.c_str());

	auto file = FS.open(filename.c_str(), "r");
	if (file) {
		cbor::Reader reader{file};

		if (!cbor::expectValue(reader, cbor::DataType::kTag, cbor::kSelfDescribeTag))
			file.seek(0);

		auto result = load(reader);

		switch (result) {
		case Result::FULL:
			logger_.err(F("Profile file %s contains too many entries (truncated)"), filename.c_str());
			break;

		case Result::NOT_FOUND:
		case Result::OUT_OF_RANGE:
		case Result::PARSE_ERROR:
		case Result::IO_ERROR:
			logger_.err(F("Profile file %s contains invalid data that has been ignored"), filename.c_str());
			break;

		case Result::OK:
		default:
			break;
		}

		return result;
	} else {
		logger_.log(automatic ? Level::DEBUG : Level::ERR,
			F("Unable to open profile file %s for reading"), filename.c_str());
		return Result::IO_ERROR;
	}
}

LEDProfile::Result inline LEDProfile::load(cbor::Reader &reader) {
	Result result = Result::PARSE_ERROR;
	uint64_t entries;
	bool indefinite;

	if (!cbor::expectArray(reader, &entries, &indefinite) || indefinite) {
		if (VERBOSE)
			logger_.trace(F("File does not contain a definite length array"));
		return Result::PARSE_ERROR;
	}

	ratios_.clear();
	result = load_ratio_configs(reader, entries);
	modified_ = result != Result::OK;

	return result;
}

LEDProfile::Result inline LEDProfile::load_ratio_configs(cbor::Reader &reader, uint64_t entries) {
	Result result = OK;

	while (entries-- > 0) {
		if (downgrade_result(result, load_ratio_config(reader)))
			return result;
	}

	return result;
}

LEDProfile::Result inline LEDProfile::load_ratio_config(cbor::Reader &reader) {
	Result result = OK;
	index_t index;
	Ratio ratio;

	if (!cbor::expectValue(reader, cbor::DataType::kArray, 2)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config entry is not an array of 2 elements"));

		add_default();
		downgrade_result(result, Result::PARSE_ERROR);
		return result;
	}

	if (downgrade_result(result, get_ratio_config_index(reader, index)) != Result::OK) {
		add_default();
		return result;
	}

	if (downgrade_result(result, get_ratio_config_ratio(reader, ratio)) != Result::OK) {
		add(index, DEFAULT_RATIO);
		return result;
	}

	downgrade_result(result, add(index, ratio));

	return result;
}

LEDProfile::Result inline LEDProfile::get_ratio_config_index(cbor::Reader &reader, index_t &index) {
	int64_t value;

	if (!cbor::expectInt(reader, &value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config index is not an int"));

		return Result::PARSE_ERROR;
	}

	if (!valid_index(value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config index %" PRIu64 " is out of range"), value);
		return Result::PARSE_ERROR;
	}

	index = (index_t)value;
	return Result::OK;
}

LEDProfile::Result inline LEDProfile::get_ratio_config_ratio(cbor::Reader &reader, Ratio &ratio) {
	Result result = OK;

	if (!cbor::expectValue(reader, cbor::DataType::kArray, 3)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio is not an array of 3 elements"));

		return Result::PARSE_ERROR;
	}

	for (uint_fast8_t i = 0; i < 3; i++) {
		if (downgrade_result(result,
				get_ratio_config_ratio_value(reader, ratio.v[i])) != Result::OK)
			return result;
	}

	return Result::OK;
}

LEDProfile::Result inline LEDProfile::get_ratio_config_ratio_value(
		cbor::Reader &reader, uint8_t &ratio_value) {
	int64_t value;

	if (!cbor::expectInt(reader, &value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio value is not an int"));

		return Result::PARSE_ERROR;
	}

	if (!valid_value(value)) {
		if (VERBOSE)
			logger_.trace(F("Ratio config ratio value %" PRId64 " is out of range"), value);
		return Result::PARSE_ERROR;
	}

	ratio_value = (uint8_t)value;
	return Result::OK;
}

LEDProfile::Result LEDProfile::save(const char *bus_name, const char *profile_name) {
	auto filename = make_filename(bus_name, profile_name);
	std::shared_lock data_lock{data_mutex_};

	logger_.notice(F("Writing profile to file %s"), filename.c_str());

	auto file = FS.open(filename.c_str(), "w", true);
	if (!file) {
		logger_.err(F("Unable to open profile file %s for writing"), filename.c_str());
		return Result::IO_ERROR;
	}

	cbor::Writer writer{file};
	bool save_default = !ratios_.empty() && ratios_.begin()->first != 0;

	writer.writeTag(cbor::kSelfDescribeTag);
	writer.beginArray(ratios_.size() + (save_default ? 1 : 0));

	if (save_default)
		save(writer, 0, DEFAULT_RATIO);

	for (auto &entry : ratios_)
		save(writer, entry.first, entry.second);

	if (file.getWriteError()) {
		logger_.err(F("Failed to write profile file %s: %u"), filename.c_str(), file.getWriteError());
		file.close();
		FS.remove(filename.c_str());
		return Result::IO_ERROR;
	} else {
		modified_ = false;
		return Result::OK;
	}
}

void LEDProfile::save(cbor::Writer &writer, index_t index, const Ratio &ratio) {
	writer.beginArray(2);
	writer.writeUnsignedInt(index);
	writer.beginArray(3);
	writer.writeUnsignedInt(ratio.r);
	writer.writeUnsignedInt(ratio.g);
	writer.writeUnsignedInt(ratio.b);
}

} // namespace aurcor
