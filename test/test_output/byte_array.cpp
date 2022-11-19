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

static void normal_rgb_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray())
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_2() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_3() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_4() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_5() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_6() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rgb_7() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)))
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_1_forward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_1_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_2_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_2_forward_2() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_2_backward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_2_backward_2() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,1,2,3,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,1,2,3,4,5,6,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_forward_3() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_backward_1() {
	std::array<uint8_t,5*3> expected{7,8,9,1,2,3,4,5,6,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,1,2,3,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_3_backward_3() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,1,2,3,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,1,2,3,4,5,6,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_forward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,1,2,3,4,5,6,7,8,9,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_forward_4() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_backward_1() {
	std::array<uint8_t,5*3> expected{10,11,12,1,2,3,4,5,6,7,8,9,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_backward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,1,2,3,4,5,6,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_backward_3() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,1,2,3,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_4_backward_4() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_forward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_forward_4() {
	std::array<uint8_t,5*3> expected{13,14,15,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_forward_5() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_backward_1() {
	std::array<uint8_t,5*3> expected{13,14,15,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_backward_2() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_backward_4() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_5_backward_5() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,16,17,18,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,16,17,18,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_4() {
	std::array<uint8_t,5*3> expected{13,14,15,16,17,18,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_5() {
	std::array<uint8_t,5*3> expected{16,17,18,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_forward_6() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_1() {
	std::array<uint8_t,5*3> expected{16,17,18,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_2() {
	std::array<uint8_t,5*3> expected{13,14,15,16,17,18,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,16,17,18,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_4() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,16,17,18,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_5() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_6_backward_6() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,16,17,18,19,20,21,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_4() {
	std::array<uint8_t,5*3> expected{13,14,15,16,17,18,19,20,21,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_5() {
	std::array<uint8_t,5*3> expected{16,17,18,19,20,21,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_6() {
	std::array<uint8_t,5*3> expected{19,20,21,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_forward_7() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_1() {
	std::array<uint8_t,5*3> expected{19,20,21,1,2,3,4,5,6,7,8,9,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_2() {
	std::array<uint8_t,5*3> expected{16,17,18,19,20,21,1,2,3,4,5,6,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_3() {
	std::array<uint8_t,5*3> expected{13,14,15,16,17,18,19,20,21,1,2,3,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,13,14,15,16,17,18,19,20,21,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_5() {
	std::array<uint8_t,5*3> expected{7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_6() {
	std::array<uint8_t,5*3> expected{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void normal_rotate_rgb_7_backward_7() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), rotate=-7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_0() {
	std::array<uint8_t,5*3> expected{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_5() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_6() {
	std::array<uint8_t,5*3> expected{16,17,18,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rgb_7() {
	std::array<uint8_t,5*3> expected{19,20,21,16,17,18,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_1_forward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_1_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 1*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_2_forward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_2_forward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_2_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,4,5,6,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_2_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,0,0,0,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 2*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_forward_1() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_forward_2() {
	std::array<uint8_t,5*3> expected{1,2,3,7,8,9,4,5,6,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_forward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,7,8,9,4,5,6,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,7,8,9,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_3_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,0,0,0,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 3*3+1)), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_forward_1() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_forward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,10,11,12,7,8,9,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_forward_3() {
	std::array<uint8_t,5*3> expected{1,2,3,10,11,12,7,8,9,4,5,6,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_forward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,10,11,12,7,8,9,4,5,6,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,10,11,12,7,8,9,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,10,11,12,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_4_backward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,0,0,0};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 4*3+1)), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_forward_1() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_forward_2() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_forward_3() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_forward_4() {
	std::array<uint8_t,5*3> expected{1,2,3,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_forward_5() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_backward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_5_backward_5() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 5*3+1)), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_1() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_2() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,16,17,18,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_4() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,16,17,18,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_5() {
	std::array<uint8_t,5*3> expected{1,2,3,16,17,18,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_forward_6() {
	std::array<uint8_t,5*3> expected{16,17,18,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,16,17,18,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,16,17,18,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,16,17,18,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_5() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_6_backward_6() {
	std::array<uint8_t,5*3> expected{16,17,18,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 6*3+1)), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_1() {
	std::array<uint8_t,5*3> expected{16,17,18,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_2() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_3() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,19,20,21};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_4() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,19,20,21,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_5() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,19,20,21,16,17,18,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_6() {
	std::array<uint8_t,5*3> expected{1,2,3,19,20,21,16,17,18,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_forward_7() {
	std::array<uint8_t,5*3> expected{19,20,21,16,17,18,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=7)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_1() {
	std::array<uint8_t,5*3> expected{1,2,3,19,20,21,16,17,18,13,14,15,10,11,12};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-1)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_2() {
	std::array<uint8_t,5*3> expected{4,5,6,1,2,3,19,20,21,16,17,18,13,14,15};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-2)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_3() {
	std::array<uint8_t,5*3> expected{7,8,9,4,5,6,1,2,3,19,20,21,16,17,18};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-3)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_4() {
	std::array<uint8_t,5*3> expected{10,11,12,7,8,9,4,5,6,1,2,3,19,20,21};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-4)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_5() {
	std::array<uint8_t,5*3> expected{13,14,15,10,11,12,7,8,9,4,5,6,1,2,3};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-5)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_6() {
	std::array<uint8_t,5*3> expected{16,17,18,13,14,15,10,11,12,7,8,9,4,5,6};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-6)
)python")->outputs_[0].data(), expected.size());
}

static void reverse_rotate_rgb_7_backward_7() {
	std::array<uint8_t,5*3> expected{19,20,21,16,17,18,13,14,15,10,11,12,7,8,9};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), TestMicroPython::run_bus(expected.size(), 1, R"python(
import aurcor; aurcor.output_rgb(bytearray(range(1, 7*3+1)), reverse=True, rotate=-7)
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
	RUN_TEST(normal_rgb_7);

	RUN_TEST(normal_rotate_rgb_1_forward_1);
	RUN_TEST(normal_rotate_rgb_1_backward_1);
	RUN_TEST(normal_rotate_rgb_2_forward_1);
	RUN_TEST(normal_rotate_rgb_2_forward_2);
	RUN_TEST(normal_rotate_rgb_2_backward_1);
	RUN_TEST(normal_rotate_rgb_2_backward_2);
	RUN_TEST(normal_rotate_rgb_3_forward_1);
	RUN_TEST(normal_rotate_rgb_3_forward_2);
	RUN_TEST(normal_rotate_rgb_3_forward_3);
	RUN_TEST(normal_rotate_rgb_3_backward_1);
	RUN_TEST(normal_rotate_rgb_3_backward_2);
	RUN_TEST(normal_rotate_rgb_3_backward_3);
	RUN_TEST(normal_rotate_rgb_4_forward_1);
	RUN_TEST(normal_rotate_rgb_4_forward_2);
	RUN_TEST(normal_rotate_rgb_4_forward_3);
	RUN_TEST(normal_rotate_rgb_4_forward_4);
	RUN_TEST(normal_rotate_rgb_4_backward_1);
	RUN_TEST(normal_rotate_rgb_4_backward_2);
	RUN_TEST(normal_rotate_rgb_4_backward_3);
	RUN_TEST(normal_rotate_rgb_4_backward_4);
	RUN_TEST(normal_rotate_rgb_5_forward_1);
	RUN_TEST(normal_rotate_rgb_5_forward_2);
	RUN_TEST(normal_rotate_rgb_5_forward_3);
	RUN_TEST(normal_rotate_rgb_5_forward_4);
	RUN_TEST(normal_rotate_rgb_5_forward_5);
	RUN_TEST(normal_rotate_rgb_5_backward_1);
	RUN_TEST(normal_rotate_rgb_5_backward_2);
	RUN_TEST(normal_rotate_rgb_5_backward_3);
	RUN_TEST(normal_rotate_rgb_5_backward_4);
	RUN_TEST(normal_rotate_rgb_5_backward_5);
	RUN_TEST(normal_rotate_rgb_6_forward_1);
	RUN_TEST(normal_rotate_rgb_6_forward_2);
	RUN_TEST(normal_rotate_rgb_6_forward_3);
	RUN_TEST(normal_rotate_rgb_6_forward_4);
	RUN_TEST(normal_rotate_rgb_6_forward_5);
	RUN_TEST(normal_rotate_rgb_6_forward_6);
	RUN_TEST(normal_rotate_rgb_6_backward_1);
	RUN_TEST(normal_rotate_rgb_6_backward_2);
	RUN_TEST(normal_rotate_rgb_6_backward_3);
	RUN_TEST(normal_rotate_rgb_6_backward_4);
	RUN_TEST(normal_rotate_rgb_6_backward_5);
	RUN_TEST(normal_rotate_rgb_6_backward_6);
	RUN_TEST(normal_rotate_rgb_7_forward_1);
	RUN_TEST(normal_rotate_rgb_7_forward_2);
	RUN_TEST(normal_rotate_rgb_7_forward_3);
	RUN_TEST(normal_rotate_rgb_7_forward_4);
	RUN_TEST(normal_rotate_rgb_7_forward_5);
	RUN_TEST(normal_rotate_rgb_7_forward_6);
	RUN_TEST(normal_rotate_rgb_7_forward_7);
	RUN_TEST(normal_rotate_rgb_7_backward_1);
	RUN_TEST(normal_rotate_rgb_7_backward_2);
	RUN_TEST(normal_rotate_rgb_7_backward_3);
	RUN_TEST(normal_rotate_rgb_7_backward_4);
	RUN_TEST(normal_rotate_rgb_7_backward_5);
	RUN_TEST(normal_rotate_rgb_7_backward_6);
	RUN_TEST(normal_rotate_rgb_7_backward_7);

	RUN_TEST(reverse_rgb_0);
	RUN_TEST(reverse_rgb_1);
	RUN_TEST(reverse_rgb_2);
	RUN_TEST(reverse_rgb_3);
	RUN_TEST(reverse_rgb_4);
	RUN_TEST(reverse_rgb_5);
	RUN_TEST(reverse_rgb_6);
	RUN_TEST(reverse_rgb_7);

	RUN_TEST(reverse_rotate_rgb_1_forward_1);
	RUN_TEST(reverse_rotate_rgb_1_backward_1);
	RUN_TEST(reverse_rotate_rgb_2_forward_1);
	RUN_TEST(reverse_rotate_rgb_2_forward_2);
	RUN_TEST(reverse_rotate_rgb_2_backward_1);
	RUN_TEST(reverse_rotate_rgb_2_backward_2);
	RUN_TEST(reverse_rotate_rgb_3_forward_1);
	RUN_TEST(reverse_rotate_rgb_3_forward_2);
	RUN_TEST(reverse_rotate_rgb_3_forward_3);
	RUN_TEST(reverse_rotate_rgb_3_backward_1);
	RUN_TEST(reverse_rotate_rgb_3_backward_2);
	RUN_TEST(reverse_rotate_rgb_3_backward_3);
	RUN_TEST(reverse_rotate_rgb_4_forward_1);
	RUN_TEST(reverse_rotate_rgb_4_forward_2);
	RUN_TEST(reverse_rotate_rgb_4_forward_3);
	RUN_TEST(reverse_rotate_rgb_4_forward_4);
	RUN_TEST(reverse_rotate_rgb_4_backward_1);
	RUN_TEST(reverse_rotate_rgb_4_backward_2);
	RUN_TEST(reverse_rotate_rgb_4_backward_3);
	RUN_TEST(reverse_rotate_rgb_4_backward_4);
	RUN_TEST(reverse_rotate_rgb_5_forward_1);
	RUN_TEST(reverse_rotate_rgb_5_forward_2);
	RUN_TEST(reverse_rotate_rgb_5_forward_3);
	RUN_TEST(reverse_rotate_rgb_5_forward_4);
	RUN_TEST(reverse_rotate_rgb_5_forward_5);
	RUN_TEST(reverse_rotate_rgb_5_backward_1);
	RUN_TEST(reverse_rotate_rgb_5_backward_2);
	RUN_TEST(reverse_rotate_rgb_5_backward_3);
	RUN_TEST(reverse_rotate_rgb_5_backward_4);
	RUN_TEST(reverse_rotate_rgb_5_backward_5);
	RUN_TEST(reverse_rotate_rgb_6_forward_1);
	RUN_TEST(reverse_rotate_rgb_6_forward_2);
	RUN_TEST(reverse_rotate_rgb_6_forward_3);
	RUN_TEST(reverse_rotate_rgb_6_forward_4);
	RUN_TEST(reverse_rotate_rgb_6_forward_5);
	RUN_TEST(reverse_rotate_rgb_6_forward_6);
	RUN_TEST(reverse_rotate_rgb_6_backward_1);
	RUN_TEST(reverse_rotate_rgb_6_backward_2);
	RUN_TEST(reverse_rotate_rgb_6_backward_3);
	RUN_TEST(reverse_rotate_rgb_6_backward_4);
	RUN_TEST(reverse_rotate_rgb_6_backward_5);
	RUN_TEST(reverse_rotate_rgb_6_backward_6);
	RUN_TEST(reverse_rotate_rgb_7_forward_1);
	RUN_TEST(reverse_rotate_rgb_7_forward_2);
	RUN_TEST(reverse_rotate_rgb_7_forward_3);
	RUN_TEST(reverse_rotate_rgb_7_forward_4);
	RUN_TEST(reverse_rotate_rgb_7_forward_5);
	RUN_TEST(reverse_rotate_rgb_7_forward_6);
	RUN_TEST(reverse_rotate_rgb_7_forward_7);
	RUN_TEST(reverse_rotate_rgb_7_backward_1);
	RUN_TEST(reverse_rotate_rgb_7_backward_2);
	RUN_TEST(reverse_rotate_rgb_7_backward_3);
	RUN_TEST(reverse_rotate_rgb_7_backward_4);
	RUN_TEST(reverse_rotate_rgb_7_backward_5);
	RUN_TEST(reverse_rotate_rgb_7_backward_6);
	RUN_TEST(reverse_rotate_rgb_7_backward_7);

	return UNITY_END();
}
