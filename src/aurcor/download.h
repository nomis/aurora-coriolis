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

#include <memory>
#include <string>
#include <unordered_set>

#include <uuid/log.h>

#include "app.h"
#include "led_profiles.h"
#include "memory_pool.h"
#include "web_client.h"

namespace aurcor {

class Download: public std::enable_shared_from_this<Download> {
public:
	static constexpr size_t MAX_FILE_SIZE = 64 * 1024;

	static void init();

	Download(App &app, const std::string &url);

	std::weak_ptr<Download> start();

	static std::shared_ptr<MemoryPool> buffers_;

private:
	static std::string filename_without_extension(const std::string &path, const std::string &extension);

	void run(std::shared_ptr<Download> self);
	void download_buses(const std::string &path);
	void download_presets(const std::string &path);
	void download_profiles(const std::string &path);
	void download_scripts(const std::string &path);

	bool bus_and_profile_from_filename(const std::string &path,
		std::string &bus_name, enum led_profile_id &profile_id);
	ssize_t download_to_buffer(const std::string &url);
	bool update_file(const std::string &filename, const std::string &url);

	static uuid::log::Logger logger_;

	App &app_;
	std::string url_;
	std::unique_ptr<MemoryBlock> buffer_;
	WebClient client_;
	std::unordered_set<std::string> changed_buses_;
	std::unordered_set<std::string> changed_presets_;
	std::unordered_set<std::pair<std::string,enum led_profile_id>,BusLEDProfileHash> changed_profiles_;
	std::unordered_set<std::string> changed_scripts_;
};

} // namespace aurcor