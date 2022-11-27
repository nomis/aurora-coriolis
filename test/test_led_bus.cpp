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

#include "test_led_bus.h"

#include <cstring>
#include <vector>

#include "aurcor/led_bus.h"
#include "aurcor/led_profiles.h"

TestByteBufferLEDBus::TestByteBufferLEDBus() : aurcor::ByteBufferLEDBus("test") {
	auto &p = profile(LED_PROFILE_NORMAL);
	p.clear();
	p.set(0, 255, 255, 255);
}

void TestByteBufferLEDBus::transmit() {
	outputs_.emplace_back(pos_, pos_ + bytes_);
	finish();
}
