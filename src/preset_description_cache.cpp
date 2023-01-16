/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2023  Simon Arlott
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

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <uuid/log.h>

#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "preset-cache";

namespace aurcor {

uuid::log::Logger PresetDescriptionCache::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

PresetDescriptionCache::PresetDescriptionCache(App &app) : app_(app) {
	logger_.trace(F("Creating preset description cache"));

	start_ = current_time_us();
	presets_ = std::make_unique<std::vector<std::string>>(Preset::names());

	for (auto &name : *presets_)
		descriptions_.emplace(name, "");
}

void PresetDescriptionCache::loop() {
	if (!presets_)
		return;

	if (!presets_->empty()) {
		auto &name = presets_->back();
		auto preset = std::make_shared<Preset>(app_, nullptr, name);

		if (preset->load() == Result::OK)
			descriptions_.insert_or_assign(name, preset->description());

		presets_->pop_back();
	}

	if (presets_->empty()) {
		presets_.reset();

		if (refresh_ == 0) {
			logger_.trace(F("Created preset description cache (%zu entries in %" PRIu64 "ms)"),
				descriptions_.size(), (current_time_us() - start_) / 1000);
		} else {
			logger_.trace(F("Updated preset description cache (%zu entries in %" PRIu64 "ms)"),
				refresh_, (current_time_us() - start_) / 1000);
			refresh_ = 0;
		}
	}
}

void PresetDescriptionCache::add(const Preset &preset) {
	auto res = descriptions_.insert_or_assign(preset.name(), preset.description());

	if (res.second) {
		logger_.trace(F("Added description of preset %s to cache"), preset.name().c_str());
	} else {
		logger_.trace(F("Updated description of preset %s in cache"), preset.name().c_str());
	}
}

void PresetDescriptionCache::add(const std::string &name) {
	auto preset = std::make_shared<Preset>(app_, nullptr, name);

	if (preset->load() == Result::OK)
		add(*preset);
}

void PresetDescriptionCache::refresh(const std::unordered_set<std::string> &names) {
	if (!names.empty()) {
		if (presets_) {
			presets_->insert(presets_->end(), names.begin(), names.end());
		} else {
			start_ = current_time_us();
			presets_ = std::make_unique<std::vector<std::string>>(names.begin(), names.end());
			refresh_ = names.size();
		}
	}
}

void PresetDescriptionCache::remove(const std::string &name) {
	auto it = descriptions_.find(name);

	if (it != descriptions_.end()) {
		descriptions_.erase(it);
		logger_.trace(F("Removed description of preset %s from cache"), name.c_str());
	}
}

} // namespace aurcor
