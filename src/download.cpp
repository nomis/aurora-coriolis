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

#include "aurcor/download.h"

#include <Arduino.h>

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

std::weak_ptr<Download> Download::start() {
	auto self = shared_from_this();

	std::thread{std::bind(&Download::run, this, self)}.detach();

	return self;
}

void Download::run(std::shared_ptr<Download> self) {
	buffer_ = buffers_->allocate(false);

	if (!buffer_) {
		logger_.err("No file buffer available for download");
		return;
	}

	logger_.debug("Downloading from %s", url_.c_str());

	std::unordered_map<std::string,std::function<void(const std::string &path)>> types{
		{"buses/", std::bind(&Download::download_buses, this, std::placeholders::_1)},
		{"profiles/", std::bind(&Download::download_profiles, this, std::placeholders::_1)},
		{"scripts/", std::bind(&Download::download_scripts, this, std::placeholders::_1)},
		{"presets/", std::bind(&Download::download_presets, this, std::placeholders::_1)},
	};

	yield();
	auto urls = client_.list_urls(url_, [&types] (const std::string &path) -> bool {
		return types.find(path) != types.end();
	});

	for (auto &url : urls) {
		auto type = types.find(url);

		if (type != types.end()) {
			yield();
			type->second(url);
		}
	}

	logger_.debug("Download complete");

	app_.refresh_files(changed_buses_, changed_presets_, changed_profiles_, changed_scripts_);
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

	auto urls = client_.list_urls(url_ + path, [this] (const std::string &path) -> bool {
		return !!app_.bus(filename_without_extension(path, LEDBusConfig::FILENAME_EXT));
	});

	for (auto &url : urls) {
		logger_.trace("Download bus config: %s", url.c_str());
		if (update_file(std::string{LEDBusConfig::DIRECTORY_NAME} + "/" + url, url_ + path + url))
			changed_buses_.insert(filename_without_extension(url, LEDBusConfig::FILENAME_EXT));
	}
}

void Download::download_presets(const std::string &path) {
	logger_.debug("Download presets");

	auto urls = client_.list_urls(url_ + path, [] (const std::string &path) -> bool {
		auto name = filename_without_extension(path, Preset::FILENAME_EXT);
		return allowed_file_name(name) && name.length() < Preset::MAX_NAME_LENGTH;
	});

	for (auto &url : urls) {
		logger_.trace("Download preset: %s", url.c_str());
		if (update_file(std::string{Preset::DIRECTORY_NAME} + "/" + url, url_ + path + url))
			changed_presets_.insert(filename_without_extension(url, Preset::FILENAME_EXT));
	}
}

bool Download::bus_and_profile_from_filename(const std::string &path,
		std::string &bus_name, enum led_profile_id &profile_id) {
	auto filename = filename_without_extension(path, LEDProfile::FILENAME_EXT);
	auto pos = filename.find('.', 1);

	if (pos == std::string::npos)
		return false;

	bus_name = filename.substr(0, pos);
	auto profile_name = filename.substr(pos + 1);

	if (!app_.bus(bus_name))
		return false;

	return LEDProfiles::lc_id(profile_name, profile_id);
}

void Download::download_profiles(const std::string &path) {
	logger_.debug("Download bus profiles");

	auto urls = client_.list_urls(url_ + path, [this] (const std::string &path) -> bool {
		std::string bus_name;
		enum led_profile_id profile_id;

		return bus_and_profile_from_filename(path, bus_name, profile_id);
	});

	for (auto &url : urls) {
		logger_.trace("Download bus profile: %s", url.c_str());
		if (update_file(std::string{LEDProfile::DIRECTORY_NAME} + "/" + url, url_ + path + url)) {
			std::string bus_name;
			enum led_profile_id profile_id;

			if (bus_and_profile_from_filename(url, bus_name, profile_id))
				changed_profiles_.insert({bus_name, profile_id});
		}
	}
}

void Download::download_scripts(const std::string &path) {
	logger_.debug("Download scripts");

	auto urls = client_.list_urls(url_ + path, [] (const std::string &path) -> bool {
		auto name = filename_without_extension(path, MicroPython::FILENAME_EXT);
		return allowed_file_name(name) && name.length() < MicroPythonFile::MAX_NAME_LENGTH;
	});

	for (auto &url : urls) {
		logger_.trace("Download script: %s", url.c_str());
		if (update_file(std::string{MicroPython::DIRECTORY_NAME} + "/" + url, url_ + path + url))
			changed_scripts_.insert(filename_without_extension(url, MicroPython::FILENAME_EXT));
	}
}

ssize_t Download::download_to_buffer(const std::string &url) {
	yield();
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

bool Download::update_file(const std::string &filename, const std::string &url) {
	ssize_t len = download_to_buffer(url);
	if (len < 0)
		return false;

	std::unique_lock lock{App::file_mutex()};
	bool changed = false;

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
		if (FS.remove(filename.c_str()))
			logger_.info("Deleted %s", filename.c_str());
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

	lock.unlock();
	yield();

	return changed;
}

} // namespace aurcor