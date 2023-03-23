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

#pragma once

#include <Arduino.h>

#include <initializer_list>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/app.h"
#include "led_profiles.h"
#include "refresh.h"

namespace aurcor {

class Download;
class LEDBus;
class MicroPython;
class Preset;
class PresetDescriptionCache;
class WebInterface;

class App: public app::App {
public:
	App();
	~App();
	void init() override;
	void start() override;
	void loop() override;

	static inline std::shared_mutex& file_mutex() { return file_mutex_; }

	inline const std::string& immutable_id() const { return app_hash(); }

	std::vector<std::string> bus_names() const;
	std::shared_ptr<LEDBus> bus(const std::string &name);
	void attach(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<MicroPython> &mp);
	bool detach(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<MicroPython> &mp = nullptr, bool clear = false);
	bool start(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<Preset> &preset, bool overwrite = true);
	std::shared_ptr<std::shared_ptr<Preset>> edit(const std::shared_ptr<LEDBus> &bus);
	bool unsaved_preset(const std::shared_ptr<LEDBus> &bus);
	void refresh(const std::string &preset_name);
	void renamed(const std::string &preset_from_name, const std::string &preset_to_name);
	inline void deleted(const std::string &preset_from_name) { renamed(preset_from_name, ""); }
	void stop(const std::shared_ptr<LEDBus> &bus);
	void restart_script(const std::shared_ptr<LEDBus> &bus);

	bool download(const std::string &url);
	void refresh_files(std::unique_ptr<Refresh> &&refresh);

	std::pair<const std::unordered_map<std::string,std::string>&,std::shared_lock<std::shared_mutex>> preset_descriptions();
	void add_preset_description(const Preset &preset);
	void add_preset_description(const std::string &name);
	void remove_preset_description(const std::string &name);

	std::string current_preset_name(std::shared_ptr<LEDBus> bus);

private:
	App(App&&) = delete;
	App(const App&) = delete;
	App& operator=(App&&) = delete;
	App& operator=(const App&) = delete;

	void add(const std::shared_ptr<LEDBus> &&bus);
	void refresh_files();
	void refresh_presets(const std::unordered_set<std::string> &preset_names);

	static std::shared_mutex file_mutex_;

	std::unordered_map<std::string,std::shared_ptr<LEDBus>> buses_;
	std::unordered_map<std::shared_ptr<LEDBus>,std::shared_ptr<MicroPython>> mps_;

	std::mutex presets_map_mutex_; // Only used to protect WebInterface
	std::unordered_map<std::shared_ptr<LEDBus>,std::shared_ptr<Preset>> presets_;

	std::unique_ptr<Download> download_;
	std::unique_ptr<WebInterface> web_interface_;

	std::shared_mutex cached_presets_mutex_;
	std::unique_ptr<PresetDescriptionCache> cached_presets_;

	std::mutex refresh_mutex_;
	std::unique_ptr<Refresh> refresh_;
};

} // namespace aurcor
