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

#include <esp_pthread.h>

#include <initializer_list>
#include <memory>
#include <vector>

#include <uuid/common.h>
#include <uuid/console.h>
#include <uuid/log.h>
#include <uuid/syslog.h>
#include <uuid/telnet.h>

#include "app/config.h"
#include "app/console.h"
#include "app/network.h"

namespace aurcor {

App::App() {

}

void App::start() {
	app::App::start();

	auto cfg = esp_pthread_get_default_config();
	cfg.stack_size = 5 * 1024;
	cfg.prio = ESP_TASK_PRIO_MIN + 1;
	cfg.inherit_cfg = true;
	esp_pthread_set_cfg(&cfg);
}

void App::loop() {
	app::App::loop();
}

} // namespace aurcor
