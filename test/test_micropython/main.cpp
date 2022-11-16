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

#include "test_led_bus.h"
#include "test_micropython.h"

static void test_do_nothing() {
	auto bus = std::make_shared<TestLEDBus>();
	TestMicroPython mp{bus};

	mp.run("");

	TEST_ASSERT_EQUAL_STRING("", mp.output_.c_str());
	TEST_ASSERT_EQUAL_INT(0, bus->outputs_.size());
	TEST_ASSERT_EQUAL_INT(0, mp.ret_);
}

static void test_stdin() {
	auto bus = std::make_shared<TestLEDBus>();
	TestMicroPython mp{bus};

	mp.run(R"python(
input()
	)python");

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"OSError: 19\r\n",
		mp.output_.c_str());
	TEST_ASSERT_EQUAL_INT(0, bus->outputs_.size());
	TEST_ASSERT_EQUAL_INT(1, mp.ret_);
}

static void test_stdout() {
	auto bus = std::make_shared<TestLEDBus>();
	TestMicroPython mp{bus};

	mp.run(R"python(
print("Hello World!")
	)python");

	TEST_ASSERT_EQUAL_STRING(
		"Hello World!\r\n",
		mp.output_.c_str());
	TEST_ASSERT_EQUAL_INT(0, bus->outputs_.size());
	TEST_ASSERT_EQUAL_INT(0, mp.ret_);
}

static void test_logging() {
	auto bus = std::make_shared<TestLEDBus>();
	TestMicroPython mp{bus};

	mp.run(R"python(
import logging
logging.info("Hello")
logging.error("World!")
	)python");

	TEST_ASSERT_EQUAL_STRING(
		"INFO:Hello\r\n"
		"ERR:World!\r\n",
		mp.output_.c_str());
	TEST_ASSERT_EQUAL_INT(0, bus->outputs_.size());
	TEST_ASSERT_EQUAL_INT(0, mp.ret_);
}

void tearDown(void) {
	TestMicroPython::tearDown();
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	TestMicroPython::init();

	RUN_TEST(test_do_nothing);
	RUN_TEST(test_stdin);
	RUN_TEST(test_stdout);
	RUN_TEST(test_logging);

	return UNITY_END();
}
