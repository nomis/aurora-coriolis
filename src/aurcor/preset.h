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
#include <shared_mutex>
#include <string>

#include <uuid/log.h>

#include "aurcor/app.h"

namespace aurcor {

class LEDBus;
class MicroPythonFile;

class Preset: public std::enable_shared_from_this<Preset> {
public:
	Preset(App &app, std::shared_ptr<LEDBus> bus, std::string name = "");
	~Preset() = default;

	std::string name() const;
	void name(const std::string &name);

	std::string script() const;
	void script(const std::string &script);

	/* Not protected by a mutex; assumes modifications only happen from one
	 * thread. Making these thread-safe would require an extra mutex to avoid
	 * blocking other readers when saving the preset.
	 */
	bool editing() const { return !editing_.expired(); }
	std::shared_ptr<std::shared_ptr<Preset>> edit();
	bool modified() const { return modified_; }

	bool load();
	void save();

	void loop();

private:
	static constexpr size_t RESTART_TIME_MS = 10000;

	static uuid::log::Logger logger_;

	bool restart() const;

	App &app_;
	std::shared_ptr<LEDBus> bus_;
	std::shared_ptr<MicroPythonFile> mp_;
	bool running_{false};
	uint64_t stop_time_ms_{0};

	mutable std::shared_mutex data_mutex_;
	std::string name_;
	std::string script_;
	bool script_changed_{false};

	std::weak_ptr<std::shared_ptr<Preset>> editing_;
	bool modified_{false};
};

} // namespace aurcor
