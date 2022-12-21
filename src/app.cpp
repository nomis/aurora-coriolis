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

#include "aurcor/app.h"

#ifndef ENV_NATIVE
# include <esp_pthread.h>
#endif

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <uuid/common.h>

#include "app/gcc.h"
#include "aurcor/constants.h"
#include "aurcor/led_bus.h"
#include "aurcor/micropython.h"
#include "aurcor/preset.h"
#include "aurcor/uart_led_bus.h"

namespace aurcor {

App::App() {

}

void App::init() {
	app::App::init();

#if defined(ARDUINO_LOLIN_S2_MINI)
	/*
	 * Reserved: Power/Boot (0 45 46) USB (19 20) Flash/SPIRAM (26 27 28 29 30 31 32)
	 * LED: 15
	 * Pull-up: 18 (10kΩ)
	 * Default: UART0 (RX-44 TX-43) UART1 (RX-18 TX-17)
	 *
	 * Usable: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 16 17 21 33 34 35 36 37 38 39 40
	 * Null: 18 41 42 43 44
	 */
	add(std::make_shared<UARTLEDBus>(0, "led0", 41, 39));
	add(std::make_shared<UARTLEDBus>(1, "led1", 42, 40));
	add(std::make_shared<NullLEDBus>("null0"));
	add(std::make_shared<NullLEDBus>("null1"));
#else
	add(std::make_shared<NullLEDBus>("null0"));
	add(std::make_shared<NullLEDBus>("null1"));
	add(std::make_shared<NullLEDBus>("null2"));
	add(std::make_shared<NullLEDBus>("null3"));
#endif

	MicroPython::setup(buses_.size());

#ifndef ENV_NATIVE
	auto cfg = esp_pthread_get_default_config();
	cfg.stack_size = MicroPython::TASK_STACK_SIZE;
	cfg.prio = uxTaskPriorityGet(nullptr);
	cfg.inherit_cfg = true;
	esp_pthread_set_cfg(&cfg);
#endif

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

		if (!preset->load()) {
			logger_.err(F("Default preset \"%s\" for bus \"%s\" not found"), preset_name.c_str(), bus->name());
			continue;
		}

		start(bus, preset);
		preset->loop();
	}
}

void App::start() {
	app::App::start();
}

void App::loop() {
	app::App::loop();

	for (auto &preset : presets_)
		preset.second->loop();
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

bool App::detach(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<MicroPython> &mp) {
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

	return true;
}

void App::start(const std::shared_ptr<LEDBus> &bus, const std::shared_ptr<Preset> &preset) {
	logger_.trace(F("Start preset \"%s\" on %s[%s]"),
		preset->name().c_str(), bus->type(), bus->name());

	auto it = presets_.find(bus);
	if (it != presets_.end())
		presets_.erase(it);

	presets_.emplace(bus, preset);
}

std::shared_ptr<std::shared_ptr<Preset>> App::edit(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);
	if (it != presets_.end())
		return it->second->edit();

	return nullptr;
}

void App::refresh(const std::string &preset_name) {
	for (auto &bus_preset : presets_) {
		auto &preset = *bus_preset.second;
		if (preset.name() == preset_name
				&& !preset.editing()
				&& !preset.modified()) {
			auto &bus = *bus_preset.first;

			logger_.debug(F("Automatically reloading preset \"%s\" on %s[%s]"),
				preset.name().c_str(), bus.type(), bus.name());
			preset.load();
		}
	}
}

void App::stop(const std::shared_ptr<LEDBus> &bus) {
	auto it = presets_.find(bus);

	if (it != presets_.end()) {
		auto &preset = *it->second;

		logger_.trace(F("Stop preset \"%s\" on %s[%s]"),
			preset.name().c_str(), bus->type(), bus->name());
		presets_.erase(it);
	}
}

} // namespace aurcor
