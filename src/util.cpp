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

#include "aurcor/util.h"

#include <cstring>
#include <shared_mutex>
#include <string>
#include <vector>

#include "app/fs.h"
#include "aurcor/app.h"

namespace aurcor {

bool allowed_file_name(const std::string &name) {
	for (auto c : name) {
		if (c >= 'a' && c <= 'z')
			continue;
		else if (c >= 'A' && c <= 'Z')
			continue;
		else if (c >= '0' && c <= '9')
			continue;
		else if (c >= '+' && c <= '.') /* +,-. */
			continue;
		else if (c == '_')
			continue;
		return false;
	}

	return !name.empty();
}

bool allowed_text(const std::string &text) {
	for (auto c : text) {
		if (c >= 0x20 && c <= 0x7E)
			continue;
		return false;
	}

	return true;
}

std::vector<std::string> list_filenames(const char *directory_name, const char *extension) {
	std::vector<std::string> names;
	const size_t extension_len = std::char_traits<char>::length(extension);
	std::shared_lock file_lock{App::file_mutex()};
	auto dir = app::FS.open(directory_name);

	if (dir && dir.isDirectory()) {
		while (1) {
			auto file = dir.openNextFile();

			if (file) {
				std::string name = file.name();

				if (name.length() > extension_len && name.find(extension, name.length() - extension_len) != std::string::npos) {
					name.resize(name.length() - extension_len);
					names.emplace_back(name);
				}
			} else {
				break;
			}
		}
	}

	return names;
}

} // namespace aurcor
