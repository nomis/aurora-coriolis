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

#include <Arduino.h>

#include <initializer_list>
#include <memory>
#include <vector>

#include <uuid/syslog.h>
#include <uuid/telnet.h>

#include "../app/app.h"
#include "../app/console.h"
#include "../app/network.h"

namespace aurcor {

class App: public app::App {
private:
#if defined(ARDUINO_LOLIN_S2_MINI)

#else
# error "Unknown board"
#endif

public:
	App();
	void start() override;
	void loop() override;

private:
};

} // namespace aurcor
