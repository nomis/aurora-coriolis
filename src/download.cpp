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

#include "aurcor/download.h"

#include <Arduino.h>

#ifndef ENV_NATIVE
# include <esp_pthread.h>
#endif

#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/fs.h"
#include "aurcor/app.h"
#include "aurcor/led_bus_config.h"
#include "aurcor/led_profiles.h"
#include "aurcor/memory_pool.h"
#include "aurcor/micropython.h"
#include "aurcor/preset.h"
#include "aurcor/refresh.h"
#include "aurcor/util.h"
#include "aurcor/web_client.h"

using app::FS;

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "download";

namespace aurcor {

uuid::log::Logger Download::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::FTP};

std::shared_ptr<MemoryPool> Download::buffers_ = std::make_shared<MemoryPool>(
	Download::MAX_FILE_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

Download::Download(App &app, const std::string &url) : app_(app), url_(url) {
	if (!url_.empty() && url_[url_.length() - 1] != '/')
		url_.push_back('/');
}

void Download::init() {
	buffers_->resize(1);
}

bool Download::start() {
	try {
#ifndef ENV_NATIVE
		auto cfg = esp_pthread_get_default_config();
		cfg.stack_size = TASK_STACK_SIZE;
		cfg.pin_to_core = ARDUINO_RUNNING_CORE;
		cfg.prio = uxTaskPriorityGet(nullptr);
		esp_pthread_set_cfg(&cfg);
#endif

		thread_ = std::thread{&Download::run, this};
		return true;
	} catch (...) {
		logger_.emerg("Out of memory");
		return false;
	}
}

bool Download::finished() {
	if (!done_)
		return false;

	thread_.join();
	return true;
}

void Download::run() {
	try {
		buffer_ = buffers_->allocate();

		if (!buffer_) {
			logger_.err("No file buffer available for download");
			done_ = true;
			return;
		}

		logger_.notice("Downloading from %s", url_.c_str());

		std::unordered_map<std::string,std::function<void(const std::string &path)>> types{
			{"buses/", std::bind(&Download::download_buses, this, std::placeholders::_1)},
			{"profiles/", std::bind(&Download::download_profiles, this, std::placeholders::_1)},
			{"scripts/", std::bind(&Download::download_scripts, this, std::placeholders::_1)},
			{"presets/", std::bind(&Download::download_presets, this, std::placeholders::_1)},
		};

		changed_ = std::make_unique<Refresh>();

		uint64_t start = current_time_us();
		auto urls = client_.list_urls(url_, [&types] (const std::string &path) -> bool {
			return types.find(path) != types.end();
		});
		download_time_ += current_time_us() - start;

		for (auto &url : urls) {
			auto type = types.find(url);

			if (type != types.end()) {
				type->second(url);
			}
		}

		logger_.notice("Download complete (http=%" PRIu64 "ms, filesystem=%" PRIu64 "ms)", download_time_ / 1000, update_time_ / 1000);

		app_.refresh_files(std::move(changed_));
		done_ = true;
	} catch (...) {
		done_ = true;
		logger_.emerg(F("Exception in download thread"));
	}
}

std::string Download::filename_without_extension(const std::string &filename, const std::string &extension) {
	if (filename.length() > extension.length() && filename.find(extension, filename.length() - extension.length()) != std::string::npos) {
		return filename.substr(0, filename.length() - extension.length());
	} else {
		return "";
	}
}

void Download::download_buses(const std::string &path) {
	logger_.debug("Download bus config");

	uint64_t start = current_time_us();
	auto urls = client_.list_urls(url_ + path, [this] (const std::string &path) -> bool {
		return !!app_.bus(filename_without_extension(path, LEDBusConfig::FILENAME_EXT));
	});
	download_time_ += current_time_us() - start;

	for (auto &url : urls) {
		logger_.trace("Download bus config: %s", url.c_str());
		if (update_file(std::string{LEDBusConfig::DIRECTORY_NAME} + "/" + url, url_ + path + url) == Update::MODIFIED)
			changed_->buses.insert(app_.bus(filename_without_extension(url, LEDBusConfig::FILENAME_EXT)));
	}
}

void Download::download_presets(const std::string &path) {
	logger_.debug("Download presets");

	uint64_t start = current_time_us();
	auto urls = client_.list_urls(url_ + path, [] (const std::string &path) -> bool {
		auto name = filename_without_extension(path, Preset::FILENAME_EXT);
		return allowed_file_name(name) && name.length() < Preset::MAX_NAME_LENGTH;
	});
	download_time_ += current_time_us() - start;

	for (auto &url : urls) {
		auto name = filename_without_extension(url, Preset::FILENAME_EXT);

		logger_.trace("Download preset: %s", url.c_str());

		switch (update_file(std::string{Preset::DIRECTORY_NAME} + "/" + url, url_ + path + url)) {
		case Update::MODIFIED:
			changed_->presets.insert(name);
			break;

		case Update::DELETED:
			app_.remove_preset_description(name);
			break;

		case Update::NO_CHANGE:
		case Update::FAILED:
		default:
			break;
		}
	}
}

bool Download::bus_and_profile_from_filename(const std::string &path,
		std::shared_ptr<LEDBus> &bus, enum led_profile_id &profile_id) {
	auto filename = filename_without_extension(path, LEDProfile::FILENAME_EXT);
	auto pos = filename.find('.', 1);

	if (pos == std::string::npos)
		return false;

	auto bus_name = filename.substr(0, pos);
	auto profile_name = filename.substr(pos + 1);

	bus = app_.bus(bus_name);
	if (!bus)
		return false;

	return LEDProfiles::lc_id(profile_name, profile_id);
}

void Download::download_profiles(const std::string &path) {
	logger_.debug("Download bus profiles");

	uint64_t start = current_time_us();
	auto urls = client_.list_urls(url_ + path, [this] (const std::string &path) -> bool {
		std::shared_ptr<LEDBus> bus;
		enum led_profile_id profile_id;

		return bus_and_profile_from_filename(path, bus, profile_id);
	});
	download_time_ += current_time_us() - start;

	for (auto &url : urls) {
		logger_.trace("Download bus profile: %s", url.c_str());
		if (update_file(std::string{LEDProfile::DIRECTORY_NAME} + "/" + url, url_ + path + url) == Update::MODIFIED) {
			std::shared_ptr<LEDBus> bus;
			enum led_profile_id profile_id;

			if (bus_and_profile_from_filename(url, bus, profile_id))
				changed_->profiles.insert({bus, profile_id});
		}
	}
}

void Download::download_scripts(const std::string &path) {
	logger_.debug("Download scripts");

	uint64_t start = current_time_us();
	auto urls = client_.list_urls(url_ + path, [] (const std::string &path) -> bool {
		auto name = filename_without_extension(path, MicroPython::FILENAME_EXT);
		return allowed_file_name(name) && name.length() < MicroPythonFile::MAX_NAME_LENGTH;
	});
	download_time_ += current_time_us() - start;

	for (auto &url : urls) {
		logger_.trace("Download script: %s", url.c_str());
		if (update_file(std::string{MicroPython::DIRECTORY_NAME} + "/" + url, url_ + path + url) == Update::MODIFIED)
			changed_->scripts.insert(filename_without_extension(url, MicroPython::FILENAME_EXT));
	}
}

ssize_t Download::download_to_buffer(const std::string &url) {
	if (!client_.open(url))
		return -1;

	ssize_t len = client_.read(reinterpret_cast<char*>(buffer_->begin()), (ssize_t)buffer_->size());

	if (len < 0)
		return -1;

	if (len == (ssize_t)buffer_->size()) {
		char c;

		ssize_t extra_len = client_.read(&c, 1);

		if (extra_len > 0)
			logger_.trace("File %s too large", url.c_str());

		if (extra_len != 0)
			return -1;
	}

	return len;
}

Download::Update Download::update_file(const std::string &filename, const std::string &url) {
	uint64_t start = current_time_us();
	ssize_t len = download_to_buffer(url);
	if (len < 0)
		return Update::FAILED;
	download_time_ += current_time_us() - start;

	std::unique_lock lock{App::file_mutex()};
	bool changed = false;
	bool deleted = false;

	start = current_time_us();

	if (len > 0) {
		auto file = FS.open(filename.c_str());

		if (file) {
			changed = (file.size() != (size_t)len);

			if (!changed) {
				uint8_t data[256];
				size_t pos = 0;
				size_t remaining = len;

				while (remaining > 0) {
					size_t block_size = std::min(sizeof(data), remaining);
					size_t read = file.read(data, block_size);

					if (read != block_size) {
						changed = true;
						break;
					}

					if (std::memcmp(buffer_->begin() + pos, data, block_size)) {
						changed = true;
						break;
					}

					pos += block_size;
					remaining -= block_size;
				}
			}
		} else {
			changed = true;
		}
	}

	if (len == 0) {
		if (FS.remove(filename.c_str())) {
			logger_.info("Deleted %s", filename.c_str());
			deleted = true;
		}
	} else if (changed) {
		auto file = FS.open(filename.c_str(), "w", true);
		size_t written = file.write(buffer_->begin(), len);

		if (written < (size_t)len) {
			logger_.err("Short write (%zu of %zu) updating %s",
				written, len, filename.c_str());
			changed = false;
		} else {
			logger_.info("Updated %s", filename.c_str());
		}
	}

	update_time_ += current_time_us() - start;

	if (deleted) {
		return Update::DELETED;
	} else if (changed) {
		return Update::MODIFIED;
	} else {
		return Update::NO_CHANGE;
	}
}

} // namespace aurcor
