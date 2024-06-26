/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2024  Simon Arlott
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

#include "aurcor/app.h"

#ifndef ENV_NATIVE
# include <driver/spi_master.h>
# include <soc/uhci_struct.h>
#endif

#include <algorithm>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <uuid/common.h>

#include "app/gcc.h"
#include "aurcor/download.h"
#include "aurcor/constants.h"
#include "aurcor/led_bus.h"
#include "aurcor/micropython.h"
#include "aurcor/preset.h"
#include "aurcor/refresh.h"
#include "aurcor/spi_led_bus.h"
#include "aurcor/uart_dma_led_bus.h"
#include "aurcor/uart_led_bus.h"
#include "aurcor/web_client.h"
#include "aurcor/web_interface.h"

namespace aurcor {

std::shared_mutex App::file_mutex_;

App::App() {
}

App::~App() {
}

void App::init() {
	app::App::init();

#if defined(ARDUINO_LOLIN_S3)
	/*
	 * Reserved: Power/Boot (0 3 45 46) USB (19 20) Flash/SPIRAM (26 27 28 29 30 31 32 33 34 35 36 37)
	 * LED: 38 (mirrored by UART0 TX!?)
	 * CH340: 43 44
	 * Default: UART0 (RX-44 TX-43) UART1 (RX-18 TX-17)
	 *
	 * Usable: 1 2 4 5 6 7 8 9 10 11 12 13 14 16 17 18 21 39 40 41 42
	 * Null: 3 45 46 47 48
	 */
	add(std::make_shared<UARTDMALEDBus>(1, &UHCI0, "led0", 45, 42));
	add(std::make_shared<UARTLEDBus>(2, "led1", 46, 41));
	//add(std::make_shared<SPILEDBus>(SPI2_HOST, "led2", 40));
	//add(std::make_shared<SPILEDBus>(SPI3_HOST, "led3", 39));
	add(std::make_shared<NullLEDBus>("null0"));
#elif defined(ARDUINO_LOLIN_S2_MINI)
	/*
	 * Reserved: Power/Boot (0 45 46) USB (19 20) Flash/SPIRAM (26 27 28 29 30 31 32)
	 * LED: 15
	 * Pull-up: 18 (10kΩ)
	 * Default: UART0 (RX-44 TX-43) UART1 (RX-18 TX-17)
	 *
	 * Usable: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 16 17 21 33 34 35 36 37 38 39 40
	 * Null: 18 41 42 43 44 45 46
	 */
	add(std::make_shared<UARTDMALEDBus>(1, &UHCI0, "led0", 45, 39));
	add(std::make_shared<UARTLEDBus>(0, "led1", 46, 37));
	//add(std::make_shared<SPILEDBus>(SPI2_HOST, "led2", 35));
	//add(std::make_shared<SPILEDBus>(SPI3_HOST, "led3", 33));
	add(std::make_shared<NullLEDBus>("null0"));
#else
	add(std::make_shared<NullLEDBus>("led0"));
	add(std::make_shared<NullLEDBus>("led1"));
	add(std::make_shared<NullLEDBus>("null0"));
	add(std::make_shared<NullLEDBus>("null1"));
#endif

	MicroPython::setup(buses_.size());
	LEDBusUDP::setup(buses_.size());

	for (auto &entry : buses_) {
		auto &bus = entry.second;
		auto preset_name = bus->default_preset();

		if (preset_name.empty())
			continue;

		auto preset = std::make_shared<Preset>(*this, bus);

		if (!preset->name(preset_name)) {
			logger_.err(F("Default preset \"%s\" for bus \"%s\" is invalid"), preset_name.c_str(), bus->name());
			continue;
		}

		if (preset->load() != Result::OK) {
			logger_.err(F("Default preset \"%s\" for bus \"%s\" unavailable"), preset_name.c_str(), bus->name());
			continue;
		}

		start(bus, preset);
		preset->loop();
	}
}

void App::start() {
	app::App::start();
#ifdef ENV_NATIVE
	WebClient::init();
#endif
	Download::init();

	preset_descriptions();

	web_interface_ = std::make_unique<WebInterface>(*this);
}

std::pair<const std::unordered_map<std::string,std::string>&,std::shared_lock<std::shared_mutex>> App::preset_descriptions() {
	std::shared_lock read_lock{cached_presets_mutex_};

	if (!cached_presets_) {
		read_lock.unlock();

		std::unique_lock write_lock{cached_presets_mutex_};

		if (!cached_presets_)
			cached_presets_ = std::make_unique<PresetDescriptionCache>(*this);

		write_lock.unlock();
		read_lock.lock();
	}

	return {cached_presets_->descriptions(), std::move(read_lock)};
}

void App::add_preset_description(const Preset &preset) {
	std::unique_lock lock{cached_presets_mutex_};

	if (cached_presets_)
		cached_presets_->add(preset);
}

void App::add_preset_description(const std::string &name) {
	std::unique_lock lock{cached_presets_mutex_};

	if (cached_presets_)
		cached_presets_->add(name);
}

void App::remove_preset_description(const std::string &name) {
	std::unique_lock lock{cached_presets_mutex_};

	if (cached_presets_)
		cached_presets_->remove(name);
}

std::string App::current_preset_name(std::shared_ptr<LEDBus> bus) {
	std::lock_guard lock{presets_map_mutex_};
	auto it = presets_.find(bus);

	if (it == presets_.end())
		return "";

	return it->second->name();
}

void App::loop() {
	app::App::loop();

	if (download_ && download_->finished())
		download_.reset();

	for (auto &bus : buses_)
		bus.second->loop();

	refresh_files();

	for (auto &preset : presets_)
		preset.second->loop();

	std::unique_lock write_lock{cached_presets_mutex_};
	if (cached_presets_)
		cached_presets_->loop();
}

void App::add(const std::shared_ptr<LEDBus> &&bus) {
	buses_.emplace(bus->name(), bus);
}

std::vector<std::string> App::bus_names() const {
	std::vector<std::string> names;

	names.reserve(buses_.size());

	for (auto &bus : buses_)
		names.emplace_back(bus.first);

	std::sort(names.begin(), names.end());
	return names;
}

std::shared_ptr<LEDBus> App::bus(const std::string &name) {
	auto it = buses_.find(name);

	if (it != buses_.end())
		return it->second;

	return {};
}

void App::attach(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<MicroPython> &mp) {
	logger_.trace(F("Attach %s[%s] to %s[%s]"),
		mp->type(), mp->name().c_str(), bus->type(), bus->name());
	mps_.emplace(bus, mp);
}

bool App::detach(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<MicroPython> &mp, bool clear) {
	auto it = mps_.find(bus);

	if (it != mps_.end()) {
		auto &other_mp = it->second;

		if (mp && other_mp != mp)
			return false;

		if (!other_mp->stop())
			return false;

		logger_.trace(F("Detach %s[%s] from %s[%s]"),
			other_mp->type(), other_mp->name().c_str(), bus->type(), bus->name());
		mps_.erase(it);
	}

	if (clear)
		bus->clear();

	return true;
}

bool App::start(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<Preset> &preset,
		bool overwrite) {
	std::unique_lock lock{presets_map_mutex_, std::defer_lock};

	if (!overwrite && unsaved_preset(bus))
		return false;

	logger_.trace(F("Start preset \"%s\" on %s[%s]"),
		preset->name().c_str(), bus->type(), bus->name());

	auto it = presets_.find(bus);
	if (it != presets_.end()) {
		it->second->detach();

		lock.lock();
		presets_.erase(it);
	} else {
		lock.lock();
	}

	presets_.emplace(bus, preset);
	return true;
}

std::shared_ptr<std::shared_ptr<Preset>> App::edit(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);
	if (it != presets_.end())
		return it->second->edit();

	return nullptr;
}

bool App::unsaved_preset(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);

	if (it != presets_.end()) {
		auto &preset = *it->second;

		return preset.modified() || preset.editing();
	}

	return false;
}

void App::refresh(const std::string &preset_name) {
	refresh_presets({preset_name});
}

void App::refresh_presets(const std::unordered_set<std::string> &preset_names) {
	for (auto &bus_preset : presets_) {
		auto &preset = *bus_preset.second;
		if (preset_names.find(preset.name()) != preset_names.end()
				&& !preset.editing()
				&& !preset.modified()) {
			auto &bus = *bus_preset.first;

			logger_.debug(F("Automatically reloading preset \"%s\" on %s[%s]"),
				preset.name().c_str(), bus.type(), bus.name());
			preset.load();
		}
	}
}

void App::renamed(const std::string &preset_from_name, const std::string &preset_to_name) {
	refresh(preset_to_name);

	for (auto &bus_preset : presets_) {
		auto &preset = *bus_preset.second;
		if (preset.name() == preset_from_name) {
			auto &bus = *bus_preset.first;

			if (!preset_to_name.empty() && !preset.editing() && !preset.modified()) {
				logger_.trace(F("Automatically renaming preset \"%s\" on %s[%s] to \"%s\""),
					preset.name().c_str(), bus.type(), bus.name(), preset_to_name.c_str());
				preset.name(preset_to_name);
				preset.modified(false);
			} else {
				logger_.trace(F("Automatically marking preset \"%s\" on %s[%s] as modified (deleted)"),
					preset.name().c_str(), bus.type(), bus.name());
				preset.modified(true);
			}
		}
	}
}

void App::stop(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);

	if (it != presets_.end()) {
		auto &preset = *it->second;

		logger_.trace(F("Stop preset \"%s\" on %s[%s]"),
			preset.name().c_str(), bus->type(), bus->name());
		preset.detach();

		std::lock_guard lock{presets_map_mutex_};
		presets_.erase(it);
	}
}

void App::restart_script(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);

	if (it != presets_.end()) {
		auto &preset = *it->second;

		logger_.trace(F("Restart script \"%s\" for \"%s\" on %s[%s]"),
			preset.script().c_str(), preset.name().c_str(),
			bus->type(), bus->name());
		preset.restart_script();
	}
}

bool App::download(const std::string &url) {
	if (download_)
		return false;

	download_ = std::make_unique<Download>(*this, url);
	if (!download_->start())
		download_.reset();
	return true;
}

void App::refresh_files(std::unique_ptr<Refresh> &&refresh) {
	std::lock_guard lock{refresh_mutex_};
	refresh_ = std::move(refresh);
}

void App::refresh_files() {
	std::lock_guard lock{refresh_mutex_};

	if (!refresh_)
		return;

	for (auto &bus : refresh_->buses) {
		logger_.trace(F("Reload config on %s[%s]"), bus->type(), bus->name());
		bus->reload_config();
	}

	for (auto &entry : refresh_->profiles) {
		auto &bus = *entry.first;
		auto profile = entry.second;

		if (bus.profile_loaded(profile)
				&& !bus.profile(profile).modified()) {
			logger_.trace(F("Reload profile \"%s\" on %s[%s]"),
				LEDProfiles::lc_name(profile), bus.type(), bus.name());
			bus.load_profile(profile);
		}
	}

	for (auto &bus_preset : presets_) {
		auto &bus = *bus_preset.first;
		auto &preset = *bus_preset.second;
		auto present_name = preset.name();

		if (refresh_->presets.find(present_name) == refresh_->presets.end()
				&& preset.uses_scripts(refresh_->scripts)) {
			logger_.trace(F("Restart script \"%s\" for \"%s\" on %s[%s]"),
				preset.script().c_str(), present_name.c_str(),
				bus.type(), bus.name());
			preset.restart_script();
		}
	}

	refresh_presets(refresh_->presets);

	{
		std::unique_lock cache_lock{cached_presets_mutex_};

		if (cached_presets_)
			cached_presets_->refresh(refresh_->presets);
	}

	refresh_.reset();
	logger_.notice("Refresh complete");
}

} // namespace aurcor
