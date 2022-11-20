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

// (230,175,11), (112,204,20), (27,179,65), (31,153,153), (32,56,128), (66,31,102), (77,27,64)

static void normal_hsv_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(0))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_2() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_3() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_4() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_5() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_6() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6))
)python")->outputs_[0].data(), expected.size());
}

static void normal_hsv_7() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7))
)python")->outputs_[0].data(), expected.size());
}

#ifndef COMMON_HSV_GENERATOR
static void normal_rotate_hsv_1_forward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_1_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_2_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_2_forward_2() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_2_backward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_2_backward_2() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,230,175,11,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,230,175,11,112,204,20,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_forward_3() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_backward_1() {
	std::array<uint8_t,5*3> expected{27,179,65,230,175,11,112,204,20,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,230,175,11,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_3_backward_3() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,230,175,11,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,230,175,11,112,204,20,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_forward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,230,175,11,112,204,20,27,179,65,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_forward_4() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_backward_1() {
	std::array<uint8_t,5*3> expected{31,153,153,230,175,11,112,204,20,27,179,65,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_backward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,230,175,11,112,204,20,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_backward_3() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,230,175,11,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_4_backward_4() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_forward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_forward_4() {
	std::array<uint8_t,5*3> expected{32,56,128,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_forward_5() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_backward_1() {
	std::array<uint8_t,5*3> expected{32,56,128,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_backward_2() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_backward_4() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_5_backward_5() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,66,31,102,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,66,31,102,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_4() {
	std::array<uint8_t,5*3> expected{32,56,128,66,31,102,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_5() {
	std::array<uint8_t,5*3> expected{66,31,102,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_forward_6() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_1() {
	std::array<uint8_t,5*3> expected{66,31,102,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_2() {
	std::array<uint8_t,5*3> expected{32,56,128,66,31,102,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,66,31,102,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_4() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,66,31,102,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_5() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_6_backward_6() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,66,31,102,77,27,64};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,66,31,102,77,27,64,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_4() {
	std::array<uint8_t,5*3> expected{32,56,128,66,31,102,77,27,64,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_5() {
	std::array<uint8_t,5*3> expected{66,31,102,77,27,64,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_6() {
	std::array<uint8_t,5*3> expected{77,27,64,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_forward_7() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_1() {
	std::array<uint8_t,5*3> expected{77,27,64,230,175,11,112,204,20,27,179,65,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_2() {
	std::array<uint8_t,5*3> expected{66,31,102,77,27,64,230,175,11,112,204,20,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_3() {
	std::array<uint8_t,5*3> expected{32,56,128,66,31,102,77,27,64,230,175,11,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,32,56,128,66,31,102,77,27,64,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_5() {
	std::array<uint8_t,5*3> expected{27,179,65,31,153,153,32,56,128,66,31,102,77,27,64};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_6() {
	std::array<uint8_t,5*3> expected{112,204,20,27,179,65,31,153,153,32,56,128,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_hsv_7_backward_7() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,27,179,65,31,153,153,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), rotate=-7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(0), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_5() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_6() {
	std::array<uint8_t,5*3> expected{66,31,102,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_hsv_7() {
	std::array<uint8_t,5*3> expected{77,27,64,66,31,102,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_1_forward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_1_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(1), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_2_forward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_2_forward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_2_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,112,204,20,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_2_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(2), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_forward_1() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,27,179,65,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_forward_2() {
	std::array<uint8_t,5*3> expected{230,175,11,27,179,65,112,204,20,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_forward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,27,179,65,112,204,20,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,27,179,65,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_3_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(3), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_forward_1() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,31,153,153,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_forward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,31,153,153,27,179,65,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_forward_3() {
	std::array<uint8_t,5*3> expected{230,175,11,31,153,153,27,179,65,112,204,20,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_forward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,31,153,153,27,179,65,112,204,20,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,31,153,153,27,179,65,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,31,153,153,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_4_backward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(4), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_forward_1() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_forward_2() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_forward_3() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_forward_4() {
	std::array<uint8_t,5*3> expected{230,175,11,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_forward_5() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_backward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_5_backward_5() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(5), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_1() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_2() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,66,31,102,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_4() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,66,31,102,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_5() {
	std::array<uint8_t,5*3> expected{230,175,11,66,31,102,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_forward_6() {
	std::array<uint8_t,5*3> expected{66,31,102,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,66,31,102,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,66,31,102,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,66,31,102,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_5() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_6_backward_6() {
	std::array<uint8_t,5*3> expected{66,31,102,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(6), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_1() {
	std::array<uint8_t,5*3> expected{66,31,102,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_2() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_3() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,77,27,64};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_4() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,77,27,64,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_5() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,77,27,64,66,31,102,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_6() {
	std::array<uint8_t,5*3> expected{230,175,11,77,27,64,66,31,102,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_forward_7() {
	std::array<uint8_t,5*3> expected{77,27,64,66,31,102,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_1() {
	std::array<uint8_t,5*3> expected{230,175,11,77,27,64,66,31,102,32,56,128,31,153,153};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_2() {
	std::array<uint8_t,5*3> expected{112,204,20,230,175,11,77,27,64,66,31,102,32,56,128};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_3() {
	std::array<uint8_t,5*3> expected{27,179,65,112,204,20,230,175,11,77,27,64,66,31,102};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_4() {
	std::array<uint8_t,5*3> expected{31,153,153,27,179,65,112,204,20,230,175,11,77,27,64};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_5() {
	std::array<uint8_t,5*3> expected{32,56,128,31,153,153,27,179,65,112,204,20,230,175,11};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_6() {
	std::array<uint8_t,5*3> expected{66,31,102,32,56,128,31,153,153,27,179,65,112,204,20};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_hsv_7_backward_7() {
	std::array<uint8_t,5*3> expected{77,27,64,66,31,102,32,56,128,31,153,153,27,179,65};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, fn + R"python(
import aurcor; aurcor.output_hsv(fn(7), reverse=True, rotate=-7)
)python")->outputs_[0].data(), expected.size());
}
#endif

int COMMON_HSV_NAME () {
	UNITY_BEGIN();

	RUN_TEST(normal_hsv_0);
	RUN_TEST(normal_hsv_1);
	RUN_TEST(normal_hsv_2);
	RUN_TEST(normal_hsv_3);
	RUN_TEST(normal_hsv_4);
	RUN_TEST(normal_hsv_5);
	RUN_TEST(normal_hsv_6);
	RUN_TEST(normal_hsv_7);

#ifndef COMMON_HSV_GENERATOR
	RUN_TEST(normal_rotate_hsv_1_forward_1);
	RUN_TEST(normal_rotate_hsv_1_backward_1);
	RUN_TEST(normal_rotate_hsv_2_forward_1);
	RUN_TEST(normal_rotate_hsv_2_forward_2);
	RUN_TEST(normal_rotate_hsv_2_backward_1);
	RUN_TEST(normal_rotate_hsv_2_backward_2);
	RUN_TEST(normal_rotate_hsv_3_forward_1);
	RUN_TEST(normal_rotate_hsv_3_forward_2);
	RUN_TEST(normal_rotate_hsv_3_forward_3);
	RUN_TEST(normal_rotate_hsv_3_backward_1);
	RUN_TEST(normal_rotate_hsv_3_backward_2);
	RUN_TEST(normal_rotate_hsv_3_backward_3);
	RUN_TEST(normal_rotate_hsv_4_forward_1);
	RUN_TEST(normal_rotate_hsv_4_forward_2);
	RUN_TEST(normal_rotate_hsv_4_forward_3);
	RUN_TEST(normal_rotate_hsv_4_forward_4);
	RUN_TEST(normal_rotate_hsv_4_backward_1);
	RUN_TEST(normal_rotate_hsv_4_backward_2);
	RUN_TEST(normal_rotate_hsv_4_backward_3);
	RUN_TEST(normal_rotate_hsv_4_backward_4);
	RUN_TEST(normal_rotate_hsv_5_forward_1);
	RUN_TEST(normal_rotate_hsv_5_forward_2);
	RUN_TEST(normal_rotate_hsv_5_forward_3);
	RUN_TEST(normal_rotate_hsv_5_forward_4);
	RUN_TEST(normal_rotate_hsv_5_forward_5);
	RUN_TEST(normal_rotate_hsv_5_backward_1);
	RUN_TEST(normal_rotate_hsv_5_backward_2);
	RUN_TEST(normal_rotate_hsv_5_backward_3);
	RUN_TEST(normal_rotate_hsv_5_backward_4);
	RUN_TEST(normal_rotate_hsv_5_backward_5);
	RUN_TEST(normal_rotate_hsv_6_forward_1);
	RUN_TEST(normal_rotate_hsv_6_forward_2);
	RUN_TEST(normal_rotate_hsv_6_forward_3);
	RUN_TEST(normal_rotate_hsv_6_forward_4);
	RUN_TEST(normal_rotate_hsv_6_forward_5);
	RUN_TEST(normal_rotate_hsv_6_forward_6);
	RUN_TEST(normal_rotate_hsv_6_backward_1);
	RUN_TEST(normal_rotate_hsv_6_backward_2);
	RUN_TEST(normal_rotate_hsv_6_backward_3);
	RUN_TEST(normal_rotate_hsv_6_backward_4);
	RUN_TEST(normal_rotate_hsv_6_backward_5);
	RUN_TEST(normal_rotate_hsv_6_backward_6);
	RUN_TEST(normal_rotate_hsv_7_forward_1);
	RUN_TEST(normal_rotate_hsv_7_forward_2);
	RUN_TEST(normal_rotate_hsv_7_forward_3);
	RUN_TEST(normal_rotate_hsv_7_forward_4);
	RUN_TEST(normal_rotate_hsv_7_forward_5);
	RUN_TEST(normal_rotate_hsv_7_forward_6);
	RUN_TEST(normal_rotate_hsv_7_forward_7);
	RUN_TEST(normal_rotate_hsv_7_backward_1);
	RUN_TEST(normal_rotate_hsv_7_backward_2);
	RUN_TEST(normal_rotate_hsv_7_backward_3);
	RUN_TEST(normal_rotate_hsv_7_backward_4);
	RUN_TEST(normal_rotate_hsv_7_backward_5);
	RUN_TEST(normal_rotate_hsv_7_backward_6);
	RUN_TEST(normal_rotate_hsv_7_backward_7);

	RUN_TEST(reverse_hsv_0);
	RUN_TEST(reverse_hsv_1);
	RUN_TEST(reverse_hsv_2);
	RUN_TEST(reverse_hsv_3);
	RUN_TEST(reverse_hsv_4);
	RUN_TEST(reverse_hsv_5);
	RUN_TEST(reverse_hsv_6);
	RUN_TEST(reverse_hsv_7);

	RUN_TEST(reverse_rotate_hsv_1_forward_1);
	RUN_TEST(reverse_rotate_hsv_1_backward_1);
	RUN_TEST(reverse_rotate_hsv_2_forward_1);
	RUN_TEST(reverse_rotate_hsv_2_forward_2);
	RUN_TEST(reverse_rotate_hsv_2_backward_1);
	RUN_TEST(reverse_rotate_hsv_2_backward_2);
	RUN_TEST(reverse_rotate_hsv_3_forward_1);
	RUN_TEST(reverse_rotate_hsv_3_forward_2);
	RUN_TEST(reverse_rotate_hsv_3_forward_3);
	RUN_TEST(reverse_rotate_hsv_3_backward_1);
	RUN_TEST(reverse_rotate_hsv_3_backward_2);
	RUN_TEST(reverse_rotate_hsv_3_backward_3);
	RUN_TEST(reverse_rotate_hsv_4_forward_1);
	RUN_TEST(reverse_rotate_hsv_4_forward_2);
	RUN_TEST(reverse_rotate_hsv_4_forward_3);
	RUN_TEST(reverse_rotate_hsv_4_forward_4);
	RUN_TEST(reverse_rotate_hsv_4_backward_1);
	RUN_TEST(reverse_rotate_hsv_4_backward_2);
	RUN_TEST(reverse_rotate_hsv_4_backward_3);
	RUN_TEST(reverse_rotate_hsv_4_backward_4);
	RUN_TEST(reverse_rotate_hsv_5_forward_1);
	RUN_TEST(reverse_rotate_hsv_5_forward_2);
	RUN_TEST(reverse_rotate_hsv_5_forward_3);
	RUN_TEST(reverse_rotate_hsv_5_forward_4);
	RUN_TEST(reverse_rotate_hsv_5_forward_5);
	RUN_TEST(reverse_rotate_hsv_5_backward_1);
	RUN_TEST(reverse_rotate_hsv_5_backward_2);
	RUN_TEST(reverse_rotate_hsv_5_backward_3);
	RUN_TEST(reverse_rotate_hsv_5_backward_4);
	RUN_TEST(reverse_rotate_hsv_5_backward_5);
	RUN_TEST(reverse_rotate_hsv_6_forward_1);
	RUN_TEST(reverse_rotate_hsv_6_forward_2);
	RUN_TEST(reverse_rotate_hsv_6_forward_3);
	RUN_TEST(reverse_rotate_hsv_6_forward_4);
	RUN_TEST(reverse_rotate_hsv_6_forward_5);
	RUN_TEST(reverse_rotate_hsv_6_forward_6);
	RUN_TEST(reverse_rotate_hsv_6_backward_1);
	RUN_TEST(reverse_rotate_hsv_6_backward_2);
	RUN_TEST(reverse_rotate_hsv_6_backward_3);
	RUN_TEST(reverse_rotate_hsv_6_backward_4);
	RUN_TEST(reverse_rotate_hsv_6_backward_5);
	RUN_TEST(reverse_rotate_hsv_6_backward_6);
	RUN_TEST(reverse_rotate_hsv_7_forward_1);
	RUN_TEST(reverse_rotate_hsv_7_forward_2);
	RUN_TEST(reverse_rotate_hsv_7_forward_3);
	RUN_TEST(reverse_rotate_hsv_7_forward_4);
	RUN_TEST(reverse_rotate_hsv_7_forward_5);
	RUN_TEST(reverse_rotate_hsv_7_forward_6);
	RUN_TEST(reverse_rotate_hsv_7_forward_7);
	RUN_TEST(reverse_rotate_hsv_7_backward_1);
	RUN_TEST(reverse_rotate_hsv_7_backward_2);
	RUN_TEST(reverse_rotate_hsv_7_backward_3);
	RUN_TEST(reverse_rotate_hsv_7_backward_4);
	RUN_TEST(reverse_rotate_hsv_7_backward_5);
	RUN_TEST(reverse_rotate_hsv_7_backward_6);
	RUN_TEST(reverse_rotate_hsv_7_backward_7);
#endif

	return UNITY_END();
}