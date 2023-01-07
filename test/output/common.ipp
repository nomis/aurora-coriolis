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
#include <string>

#include "test_led_bus.h"
#include "test_micropython.h"

static void normal_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0))
)python")->outputs_[0].data(), expected.size());
}

static void normal_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1))
)python")->outputs_[0].data(), expected.size());
}

static void normal_2() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2))
)python")->outputs_[0].data(), expected.size());
}

static void normal_3() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3))
)python")->outputs_[0].data(), expected.size());
}

static void normal_4() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4))
)python")->outputs_[0].data(), expected.size());
}

static void normal_5() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5))
)python")->outputs_[0].data(), expected.size());
}

static void normal_6() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6))
)python")->outputs_[0].data(), expected.size());
}

static void normal_7() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7))
)python")->outputs_[0].data(), expected.size());
}

static void normal_0_repeat() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), repeat=True)
)python")->outputs_[0].data(), expected.size());
}

#ifndef COMMON_GENERATOR
static void normal_rotate_1_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_1_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_A,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_A,EXPECT_B,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_A,EXPECT_B,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_A,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_7() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_7() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-7)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_1_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_1_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_2_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_3_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_4_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_5_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_forward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_6_backward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), rotate=-6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_forward_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=7, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_7_backward_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_C,EXPECT_D,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), rotate=-7, repeat=True)
)python")->outputs_[0].data(), expected.size());
}


static void reverse_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_5() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_6() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_7() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_0_repeat() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_1_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_1_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_C,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_C,EXPECT_B,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_C,EXPECT_B,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_C,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_7() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_1() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_2() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_3() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_4() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_5() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_6() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_7() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_1_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_1_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_2_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_3_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_4_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_5_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_forward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_6_backward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, rotate=-6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_forward_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=7, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_1_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-1, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_2_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F,EXPECT_E};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-2, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_3_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G,EXPECT_F};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-3, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_4_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A,EXPECT_G};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-4, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_5_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-5, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_6_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-6, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_7_backward_7_repeat() {
	std::array<uint8_t,5*3> expected{EXPECT_G,EXPECT_F,EXPECT_E,EXPECT_D,EXPECT_C};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, rotate=-7, repeat=True)
)python")->outputs_[0].data(), expected.size());
}
#else
static void reverse_0_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_1_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_2_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_3_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_4_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_5_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_6_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_7_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_0_repeat_generator() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(0), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_1_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(1), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_2_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_B,EXPECT_A,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(2), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_3_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_B,EXPECT_A,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(3), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_4_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_A,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(4), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_5_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(5), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_6_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(6), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_7_repeat_generator() {
	std::array<uint8_t,5*3> expected{EXPECT_E,EXPECT_D,EXPECT_C,EXPECT_B,EXPECT_A};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size() / 3, 1, fn + R"python(
import aurcor; aurcor.)python" + output_fn_name + R"python((fn(7), reverse=True, repeat=True)
)python")->outputs_[0].data(), expected.size());
}
#endif

void tearDown(void) {
	TestMicroPython::tearDown();
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	TestMicroPython::init();

	RUN_TEST(normal_0);
	RUN_TEST(normal_1);
	RUN_TEST(normal_2);
	RUN_TEST(normal_3);
	RUN_TEST(normal_4);
	RUN_TEST(normal_5);
	RUN_TEST(normal_6);
	RUN_TEST(normal_7);

	RUN_TEST(normal_0_repeat);
	RUN_TEST(normal_1_repeat);
	RUN_TEST(normal_2_repeat);
	RUN_TEST(normal_3_repeat);
	RUN_TEST(normal_4_repeat);
	RUN_TEST(normal_5_repeat);
	RUN_TEST(normal_6_repeat);
	RUN_TEST(normal_7_repeat);

#ifndef COMMON_GENERATOR
	RUN_TEST(normal_rotate_1_forward_1);
	RUN_TEST(normal_rotate_1_backward_1);
	RUN_TEST(normal_rotate_2_forward_1);
	RUN_TEST(normal_rotate_2_forward_2);
	RUN_TEST(normal_rotate_2_backward_1);
	RUN_TEST(normal_rotate_2_backward_2);
	RUN_TEST(normal_rotate_3_forward_1);
	RUN_TEST(normal_rotate_3_forward_2);
	RUN_TEST(normal_rotate_3_forward_3);
	RUN_TEST(normal_rotate_3_backward_1);
	RUN_TEST(normal_rotate_3_backward_2);
	RUN_TEST(normal_rotate_3_backward_3);
	RUN_TEST(normal_rotate_4_forward_1);
	RUN_TEST(normal_rotate_4_forward_2);
	RUN_TEST(normal_rotate_4_forward_3);
	RUN_TEST(normal_rotate_4_forward_4);
	RUN_TEST(normal_rotate_4_backward_1);
	RUN_TEST(normal_rotate_4_backward_2);
	RUN_TEST(normal_rotate_4_backward_3);
	RUN_TEST(normal_rotate_4_backward_4);
	RUN_TEST(normal_rotate_5_forward_1);
	RUN_TEST(normal_rotate_5_forward_2);
	RUN_TEST(normal_rotate_5_forward_3);
	RUN_TEST(normal_rotate_5_forward_4);
	RUN_TEST(normal_rotate_5_forward_5);
	RUN_TEST(normal_rotate_5_backward_1);
	RUN_TEST(normal_rotate_5_backward_2);
	RUN_TEST(normal_rotate_5_backward_3);
	RUN_TEST(normal_rotate_5_backward_4);
	RUN_TEST(normal_rotate_5_backward_5);
	RUN_TEST(normal_rotate_6_forward_1);
	RUN_TEST(normal_rotate_6_forward_2);
	RUN_TEST(normal_rotate_6_forward_3);
	RUN_TEST(normal_rotate_6_forward_4);
	RUN_TEST(normal_rotate_6_forward_5);
	RUN_TEST(normal_rotate_6_forward_6);
	RUN_TEST(normal_rotate_6_backward_1);
	RUN_TEST(normal_rotate_6_backward_2);
	RUN_TEST(normal_rotate_6_backward_3);
	RUN_TEST(normal_rotate_6_backward_4);
	RUN_TEST(normal_rotate_6_backward_5);
	RUN_TEST(normal_rotate_6_backward_6);
	RUN_TEST(normal_rotate_7_forward_1);
	RUN_TEST(normal_rotate_7_forward_2);
	RUN_TEST(normal_rotate_7_forward_3);
	RUN_TEST(normal_rotate_7_forward_4);
	RUN_TEST(normal_rotate_7_forward_5);
	RUN_TEST(normal_rotate_7_forward_6);
	RUN_TEST(normal_rotate_7_forward_7);
	RUN_TEST(normal_rotate_7_backward_1);
	RUN_TEST(normal_rotate_7_backward_2);
	RUN_TEST(normal_rotate_7_backward_3);
	RUN_TEST(normal_rotate_7_backward_4);
	RUN_TEST(normal_rotate_7_backward_5);
	RUN_TEST(normal_rotate_7_backward_6);
	RUN_TEST(normal_rotate_7_backward_7);

	RUN_TEST(normal_rotate_1_forward_1_repeat);
	RUN_TEST(normal_rotate_1_backward_1_repeat);
	RUN_TEST(normal_rotate_2_forward_1_repeat);
	RUN_TEST(normal_rotate_2_forward_2_repeat);
	RUN_TEST(normal_rotate_2_backward_1_repeat);
	RUN_TEST(normal_rotate_2_backward_2_repeat);
	RUN_TEST(normal_rotate_3_forward_1_repeat);
	RUN_TEST(normal_rotate_3_forward_2_repeat);
	RUN_TEST(normal_rotate_3_forward_3_repeat);
	RUN_TEST(normal_rotate_3_backward_1_repeat);
	RUN_TEST(normal_rotate_3_backward_2_repeat);
	RUN_TEST(normal_rotate_3_backward_3_repeat);
	RUN_TEST(normal_rotate_4_forward_1_repeat);
	RUN_TEST(normal_rotate_4_forward_2_repeat);
	RUN_TEST(normal_rotate_4_forward_3_repeat);
	RUN_TEST(normal_rotate_4_forward_4_repeat);
	RUN_TEST(normal_rotate_4_backward_1_repeat);
	RUN_TEST(normal_rotate_4_backward_2_repeat);
	RUN_TEST(normal_rotate_4_backward_3_repeat);
	RUN_TEST(normal_rotate_4_backward_4_repeat);
	RUN_TEST(normal_rotate_5_forward_1_repeat);
	RUN_TEST(normal_rotate_5_forward_2_repeat);
	RUN_TEST(normal_rotate_5_forward_3_repeat);
	RUN_TEST(normal_rotate_5_forward_4_repeat);
	RUN_TEST(normal_rotate_5_forward_5_repeat);
	RUN_TEST(normal_rotate_5_backward_1_repeat);
	RUN_TEST(normal_rotate_5_backward_2_repeat);
	RUN_TEST(normal_rotate_5_backward_3_repeat);
	RUN_TEST(normal_rotate_5_backward_4_repeat);
	RUN_TEST(normal_rotate_5_backward_5_repeat);
	RUN_TEST(normal_rotate_6_forward_1_repeat);
	RUN_TEST(normal_rotate_6_forward_2_repeat);
	RUN_TEST(normal_rotate_6_forward_3_repeat);
	RUN_TEST(normal_rotate_6_forward_4_repeat);
	RUN_TEST(normal_rotate_6_forward_5_repeat);
	RUN_TEST(normal_rotate_6_forward_6_repeat);
	RUN_TEST(normal_rotate_6_backward_1_repeat);
	RUN_TEST(normal_rotate_6_backward_2_repeat);
	RUN_TEST(normal_rotate_6_backward_3_repeat);
	RUN_TEST(normal_rotate_6_backward_4_repeat);
	RUN_TEST(normal_rotate_6_backward_5_repeat);
	RUN_TEST(normal_rotate_6_backward_6_repeat);
	RUN_TEST(normal_rotate_7_forward_1_repeat);
	RUN_TEST(normal_rotate_7_forward_2_repeat);
	RUN_TEST(normal_rotate_7_forward_3_repeat);
	RUN_TEST(normal_rotate_7_forward_4_repeat);
	RUN_TEST(normal_rotate_7_forward_5_repeat);
	RUN_TEST(normal_rotate_7_forward_6_repeat);
	RUN_TEST(normal_rotate_7_forward_7_repeat);
	RUN_TEST(normal_rotate_7_backward_1_repeat);
	RUN_TEST(normal_rotate_7_backward_2_repeat);
	RUN_TEST(normal_rotate_7_backward_3_repeat);
	RUN_TEST(normal_rotate_7_backward_4_repeat);
	RUN_TEST(normal_rotate_7_backward_5_repeat);
	RUN_TEST(normal_rotate_7_backward_6_repeat);
	RUN_TEST(normal_rotate_7_backward_7_repeat);

	RUN_TEST(reverse_0);
	RUN_TEST(reverse_1);
	RUN_TEST(reverse_2);
	RUN_TEST(reverse_3);
	RUN_TEST(reverse_4);
	RUN_TEST(reverse_5);
	RUN_TEST(reverse_6);
	RUN_TEST(reverse_7);

	RUN_TEST(reverse_0_repeat);
	RUN_TEST(reverse_1_repeat);
	RUN_TEST(reverse_2_repeat);
	RUN_TEST(reverse_3_repeat);
	RUN_TEST(reverse_4_repeat);
	RUN_TEST(reverse_5_repeat);
	RUN_TEST(reverse_6_repeat);
	RUN_TEST(reverse_7_repeat);

	RUN_TEST(reverse_rotate_1_forward_1);
	RUN_TEST(reverse_rotate_1_backward_1);
	RUN_TEST(reverse_rotate_2_forward_1);
	RUN_TEST(reverse_rotate_2_forward_2);
	RUN_TEST(reverse_rotate_2_backward_1);
	RUN_TEST(reverse_rotate_2_backward_2);
	RUN_TEST(reverse_rotate_3_forward_1);
	RUN_TEST(reverse_rotate_3_forward_2);
	RUN_TEST(reverse_rotate_3_forward_3);
	RUN_TEST(reverse_rotate_3_backward_1);
	RUN_TEST(reverse_rotate_3_backward_2);
	RUN_TEST(reverse_rotate_3_backward_3);
	RUN_TEST(reverse_rotate_4_forward_1);
	RUN_TEST(reverse_rotate_4_forward_2);
	RUN_TEST(reverse_rotate_4_forward_3);
	RUN_TEST(reverse_rotate_4_forward_4);
	RUN_TEST(reverse_rotate_4_backward_1);
	RUN_TEST(reverse_rotate_4_backward_2);
	RUN_TEST(reverse_rotate_4_backward_3);
	RUN_TEST(reverse_rotate_4_backward_4);
	RUN_TEST(reverse_rotate_5_forward_1);
	RUN_TEST(reverse_rotate_5_forward_2);
	RUN_TEST(reverse_rotate_5_forward_3);
	RUN_TEST(reverse_rotate_5_forward_4);
	RUN_TEST(reverse_rotate_5_forward_5);
	RUN_TEST(reverse_rotate_5_backward_1);
	RUN_TEST(reverse_rotate_5_backward_2);
	RUN_TEST(reverse_rotate_5_backward_3);
	RUN_TEST(reverse_rotate_5_backward_4);
	RUN_TEST(reverse_rotate_5_backward_5);
	RUN_TEST(reverse_rotate_6_forward_1);
	RUN_TEST(reverse_rotate_6_forward_2);
	RUN_TEST(reverse_rotate_6_forward_3);
	RUN_TEST(reverse_rotate_6_forward_4);
	RUN_TEST(reverse_rotate_6_forward_5);
	RUN_TEST(reverse_rotate_6_forward_6);
	RUN_TEST(reverse_rotate_6_backward_1);
	RUN_TEST(reverse_rotate_6_backward_2);
	RUN_TEST(reverse_rotate_6_backward_3);
	RUN_TEST(reverse_rotate_6_backward_4);
	RUN_TEST(reverse_rotate_6_backward_5);
	RUN_TEST(reverse_rotate_6_backward_6);
	RUN_TEST(reverse_rotate_7_forward_1);
	RUN_TEST(reverse_rotate_7_forward_2);
	RUN_TEST(reverse_rotate_7_forward_3);
	RUN_TEST(reverse_rotate_7_forward_4);
	RUN_TEST(reverse_rotate_7_forward_5);
	RUN_TEST(reverse_rotate_7_forward_6);
	RUN_TEST(reverse_rotate_7_forward_7);
	RUN_TEST(reverse_rotate_7_backward_1);
	RUN_TEST(reverse_rotate_7_backward_2);
	RUN_TEST(reverse_rotate_7_backward_3);
	RUN_TEST(reverse_rotate_7_backward_4);
	RUN_TEST(reverse_rotate_7_backward_5);
	RUN_TEST(reverse_rotate_7_backward_6);
	RUN_TEST(reverse_rotate_7_backward_7);

	RUN_TEST(reverse_rotate_1_forward_1_repeat);
	RUN_TEST(reverse_rotate_1_backward_1_repeat);
	RUN_TEST(reverse_rotate_2_forward_1_repeat);
	RUN_TEST(reverse_rotate_2_forward_2_repeat);
	RUN_TEST(reverse_rotate_2_backward_1_repeat);
	RUN_TEST(reverse_rotate_2_backward_2_repeat);
	RUN_TEST(reverse_rotate_3_forward_1_repeat);
	RUN_TEST(reverse_rotate_3_forward_2_repeat);
	RUN_TEST(reverse_rotate_3_forward_3_repeat);
	RUN_TEST(reverse_rotate_3_backward_1_repeat);
	RUN_TEST(reverse_rotate_3_backward_2_repeat);
	RUN_TEST(reverse_rotate_3_backward_3_repeat);
	RUN_TEST(reverse_rotate_4_forward_1_repeat);
	RUN_TEST(reverse_rotate_4_forward_2_repeat);
	RUN_TEST(reverse_rotate_4_forward_3_repeat);
	RUN_TEST(reverse_rotate_4_forward_4_repeat);
	RUN_TEST(reverse_rotate_4_backward_1_repeat);
	RUN_TEST(reverse_rotate_4_backward_2_repeat);
	RUN_TEST(reverse_rotate_4_backward_3_repeat);
	RUN_TEST(reverse_rotate_4_backward_4_repeat);
	RUN_TEST(reverse_rotate_5_forward_1_repeat);
	RUN_TEST(reverse_rotate_5_forward_2_repeat);
	RUN_TEST(reverse_rotate_5_forward_3_repeat);
	RUN_TEST(reverse_rotate_5_forward_4_repeat);
	RUN_TEST(reverse_rotate_5_forward_5_repeat);
	RUN_TEST(reverse_rotate_5_backward_1_repeat);
	RUN_TEST(reverse_rotate_5_backward_2_repeat);
	RUN_TEST(reverse_rotate_5_backward_3_repeat);
	RUN_TEST(reverse_rotate_5_backward_4_repeat);
	RUN_TEST(reverse_rotate_5_backward_5_repeat);
	RUN_TEST(reverse_rotate_6_forward_1_repeat);
	RUN_TEST(reverse_rotate_6_forward_2_repeat);
	RUN_TEST(reverse_rotate_6_forward_3_repeat);
	RUN_TEST(reverse_rotate_6_forward_4_repeat);
	RUN_TEST(reverse_rotate_6_forward_5_repeat);
	RUN_TEST(reverse_rotate_6_forward_6_repeat);
	RUN_TEST(reverse_rotate_6_backward_1_repeat);
	RUN_TEST(reverse_rotate_6_backward_2_repeat);
	RUN_TEST(reverse_rotate_6_backward_3_repeat);
	RUN_TEST(reverse_rotate_6_backward_4_repeat);
	RUN_TEST(reverse_rotate_6_backward_5_repeat);
	RUN_TEST(reverse_rotate_6_backward_6_repeat);
	RUN_TEST(reverse_rotate_7_forward_1_repeat);
	RUN_TEST(reverse_rotate_7_forward_2_repeat);
	RUN_TEST(reverse_rotate_7_forward_3_repeat);
	RUN_TEST(reverse_rotate_7_forward_4_repeat);
	RUN_TEST(reverse_rotate_7_forward_5_repeat);
	RUN_TEST(reverse_rotate_7_forward_6_repeat);
	RUN_TEST(reverse_rotate_7_forward_7_repeat);
	RUN_TEST(reverse_rotate_7_backward_1_repeat);
	RUN_TEST(reverse_rotate_7_backward_2_repeat);
	RUN_TEST(reverse_rotate_7_backward_3_repeat);
	RUN_TEST(reverse_rotate_7_backward_4_repeat);
	RUN_TEST(reverse_rotate_7_backward_5_repeat);
	RUN_TEST(reverse_rotate_7_backward_6_repeat);
	RUN_TEST(reverse_rotate_7_backward_7_repeat);
#else
	RUN_TEST(reverse_0_generator);
	RUN_TEST(reverse_1_generator);
	RUN_TEST(reverse_2_generator);
	RUN_TEST(reverse_3_generator);
	RUN_TEST(reverse_4_generator);
	RUN_TEST(reverse_5_generator);
	RUN_TEST(reverse_6_generator);
	RUN_TEST(reverse_7_generator);

	RUN_TEST(reverse_0_repeat_generator);
	RUN_TEST(reverse_1_repeat_generator);
	RUN_TEST(reverse_2_repeat_generator);
	RUN_TEST(reverse_3_repeat_generator);
	RUN_TEST(reverse_4_repeat_generator);
	RUN_TEST(reverse_5_repeat_generator);
	RUN_TEST(reverse_6_repeat_generator);
	RUN_TEST(reverse_7_repeat_generator);
#endif

	return UNITY_END();
}
