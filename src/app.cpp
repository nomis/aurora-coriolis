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

#include <memory>

#include "app/gcc.h"
#include "aurcor/led_bus.h"
#include "aurcor/micropython.h"
#include "aurcor/uart_led_bus.h"

namespace aurcor {

App::App() {

}

void App::init() {
	app::App::init();

#if defined(ARDUINO_LOLIN_S2_MINI)
	buses_.push_back(std::make_unique<UARTLEDBus<0>>(F("led0"),  1, 39));
	buses_.push_back(std::make_unique<UARTLEDBus<1>>(F("led1"), 14, 40));
	buses_.push_back(std::make_unique<NullLEDBus>(F("null0")));
	buses_.push_back(std::make_unique<NullLEDBus>(F("null1")));
#else
	buses_.push_back(std::make_unique<NullLEDBus>(F("null0")));
	buses_.push_back(std::make_unique<NullLEDBus>(F("null1")));
	buses_.push_back(std::make_unique<NullLEDBus>(F("null2")));
	buses_.push_back(std::make_unique<NullLEDBus>(F("null3")));
#endif

	MicroPython::setup(buses_.size());
}

void App::start() {
	app::App::start();

#ifndef ENV_NATIVE
	auto cfg = esp_pthread_get_default_config();
	cfg.stack_size = MicroPython::TASK_STACK_SIZE;
	cfg.prio = uxTaskPriorityGet(nullptr);
	cfg.inherit_cfg = true;
	esp_pthread_set_cfg(&cfg);
#endif

	// TODO make this configurable
	for (auto& bus : buses_)
		bus->length(LEDBus::MAX_LEDS);
}

void App::loop() {
	app::App::loop();
}

} // namespace aurcor
