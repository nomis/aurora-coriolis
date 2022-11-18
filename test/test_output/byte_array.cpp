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

#include <array>

#include "test_led_bus.h"
#include "test_micropython.h"

void normal_rgb_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray())
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)))
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_2() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)))
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_3() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)))
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_4() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)))
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_5() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)))
)python")->outputs_[0].data(), expected.size());
}

void normal_rgb_6() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)))
)python")->outputs_[0].data(), expected.size());
}

int testsuite_byte_array() {
	UNITY_BEGIN();

	RUN_TEST(normal_rgb_0);
	RUN_TEST(normal_rgb_1);
	RUN_TEST(normal_rgb_2);
	RUN_TEST(normal_rgb_3);
	RUN_TEST(normal_rgb_4);
	RUN_TEST(normal_rgb_5);
	RUN_TEST(normal_rgb_6);

	return UNITY_END();
}
