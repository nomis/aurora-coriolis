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

#include <Arduino.h>

#include "aurcor/app.h"

static aurcor::App application;

void setup() {
#ifndef ENV_NATIVE
	heap_caps_malloc_extmem_enable(0);
#endif

	try {
		application.start();
	} catch (...) {
#ifdef ENV_NATIVE
		throw;
#endif
		application.exception(F("setup"));
	}
}

void loop() {
	try {
		application.loop();
		::yield();
	} catch (...) {
#ifdef ENV_NATIVE
		throw;
#endif
		application.exception(F("loop"));
	}
}
