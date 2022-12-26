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

#include <unity.h>
#include <Arduino.h>

#include <memory>

#include "aurcor/app.h"
#include "aurcor/preset.h"

#include "test_led_bus.h"
#include "test_micropython.h"

static aurcor::App test_app;

static void test_save() {
	auto bus = std::make_shared<TestByteBufferLEDBus>();
	auto preset = std::make_shared<aurcor::Preset>(test_app, bus);
	TestMicroPython mp{bus, preset};

	mp.run(R"python(
import aurcor
aurcor.register_config({
	"a": ("bool", True),
	"b": ("bool", None),
	"c": ("s32", 1),
	"d": ("s32", None),
	"e": ("rgb", (1, 2, 3)),
	"f": ("rgb", None),
	"g": ("list_u16", [1, 2, 3]),
	"h": ("list_u16", None),
	"i": ("list_s32", [1, 2, 3, -4, -5 ,-6]),
	"j": ("list_s32", None),
	"k": ("list_rgb", [(1, 2, 3), (4, 5, 6), 0xFF9900]),
	"l": ("list_rgb", None),
	"m": ("set_u16", [1, 2, 3, 2, 1]),
	"n": ("set_u16", None),
	"o": ("set_s32", [1, 2, 3, 2, 1]),
	"p": ("set_s32", None),
	"q": ("set_rgb", [(1, 2, 3), (4, 5, 6), (1, 2, 3), 0xFF9900, 0x00FF00, 0xFF9900]),
	"r": ("set_rgb", None),
})
	)python");

	TEST_ASSERT_EQUAL_STRING("", mp.output_.c_str());
	TEST_ASSERT_EQUAL_INT(0, bus->outputs_.size());
	TEST_ASSERT_EQUAL_INT(0, mp.ret_);

	TEST_ASSERT_TRUE(preset->name(std::string{"test_save"}));
	TEST_ASSERT_EQUAL_INT(aurcor::Result::OK, preset->save());
	// TODO check output
}

void tearDown(void) {
	TestMicroPython::tearDown();
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	TestMicroPython::init();

	RUN_TEST(test_save);

	return UNITY_END();
}
