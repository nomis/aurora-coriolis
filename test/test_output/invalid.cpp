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

#include "test_micropython.h"

static void length_not_int() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: length must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], "1")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: length must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], 1.0)
)python")->output_.c_str());
}

static void length_below_min() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: length must be positive\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], -1)
)python")->output_.c_str());
}

static void length_above_max() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"OverflowError: overflow converting length value to bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], 2**64//3 + 1)
)python")->output_.c_str());
}

static void profile_not_int() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: profile must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], profile="NORMAL")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: profile must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], profile=0.0)
)python")->output_.c_str());
}

static void profile_below_min() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: invalid profile\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], profile=-1)
)python")->output_.c_str());
}

static void profile_above_max() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: invalid profile\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], profile=len(list(filter(lambda name: not name.startswith("_"), dir(aurcor.profiles)))))
)python")->output_.c_str());
}

static void fps_and_wait_ms() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't specify both fps and wait_ms at the same time\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], fps=1, wait_ms=1000)
)python")->output_.c_str());
}

static void fps_not_int() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: fps must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], fps="1")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: fps must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], fps=1.0)
)python")->output_.c_str());
}

static void fps_below_min() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: fps out of range\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], fps=0)
)python")->output_.c_str());
}

static void fps_above_max() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: fps out of range\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], fps=101)
)python")->output_.c_str());
}

static void wait_ms_not_int() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: wait_ms must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], wait_ms="10")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: wait_ms must be an int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], wait_ms=10.0)
)python")->output_.c_str());
}

static void wait_ms_below_min() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: wait_ms out of range\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], wait_ms=9)
)python")->output_.c_str());
}

static void wait_ms_above_max() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: wait_ms out of range\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], wait_ms=1001)
)python")->output_.c_str());
}

static void repeat_not_bool() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat=0)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat=1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat="0")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat="1")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat="False")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat="True")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat=0.0)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: repeat must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], repeat=1.0)
)python")->output_.c_str());
}

static void reverse_not_bool() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse=0)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse=1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse="0")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse="1")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse="False")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse="True")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse=0.0)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: reverse must be a bool\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([], reverse=1.0)
)python")->output_.c_str());
}

static void rotate_not_int() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: can't convert str to int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate="1")
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: can't convert float to int\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate=1.0)
)python")->output_.c_str());
}

static void rotate_below_min() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of byte array\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate=(-2**63 + 2)//3)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"OverflowError: overflow converting rotate value to bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate=(-2**63 + 2)//3 - 1)
)python")->output_.c_str());
}

static void rotate_above_max() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of byte array\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate=2**63//3)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"OverflowError: overflow converting rotate value to bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(), rotate=2**63//3 + 1)
)python")->output_.c_str());
}

static void rotate_below_length() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of byte array\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(5*3), rotate=-6)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of values\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([0] * 5, rotate=-6)
)python")->output_.c_str());
}

static void rotate_above_length() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of byte array\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(5*3), rotate=6)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: can't rotate by more than the length of values\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb([0] * 5, rotate=6)
)python")->output_.c_str());
}

static void byte_array_multiple_of_3() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: byte array length must be a multiple of 3 bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(1))
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: byte array length must be a multiple of 3 bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(2))
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: byte array length must be a multiple of 3 bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(4))
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: byte array length must be a multiple of 3 bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(5))
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"ValueError: byte array length must be a multiple of 3 bytes\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(bytearray(7))
)python")->output_.c_str());
}

static void rotate_with_unsupported_objects() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: object of type 'filter' has no len()\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(filter(None, [1, 2, 3, 4, 5]), rotate=1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: object of type 'filter' has no len()\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(filter(None, [1, 2, 3, 4, 5]), rotate=-1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: 'Test' object isn't subscriptable\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return 5;

import aurcor; aurcor.output_rgb(Test(), rotate=1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: 'Test' object isn't subscriptable\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return 5;

import aurcor; aurcor.output_rgb(Test(), rotate=-1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: can't convert str to int\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return "42";

import aurcor; aurcor.output_rgb(Test(), rotate=1)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: can't convert str to int\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return "42";

import aurcor; aurcor.output_rgb(Test(), rotate=-1)
)python")->output_.c_str());
}

static void reverse_with_unsupported_objects() {
	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 2, in <module>\r\n"
		"TypeError: object of type 'filter' has no len()\r\n",
		TestMicroPython::run_script(R"python(
import aurcor; aurcor.output_rgb(filter(None, [1, 2, 3, 4, 5]), reverse=True)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: 'Test' object isn't subscriptable\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return 5;

import aurcor; aurcor.output_rgb(Test(), reverse=True)
)python")->output_.c_str());

	TEST_ASSERT_EQUAL_STRING(
		"Traceback (most recent call last):\r\n"
		"  File \"<stdin>\", line 6, in <module>\r\n"
		"TypeError: can't convert str to int\r\n",
		TestMicroPython::run_script(R"python(
class Test:
	def __len__(self):
		return "42";

import aurcor; aurcor.output_rgb(Test(), reverse=True)
)python")->output_.c_str());
}

void testsuite_invalid() {
	RUN_TEST(length_not_int);
	RUN_TEST(length_below_min);
	RUN_TEST(length_above_max);
	RUN_TEST(profile_not_int);
	RUN_TEST(profile_below_min);
	RUN_TEST(profile_above_max);
	RUN_TEST(fps_and_wait_ms);
	RUN_TEST(fps_not_int);
	RUN_TEST(fps_below_min);
	RUN_TEST(fps_above_max);
	RUN_TEST(wait_ms_not_int);
	RUN_TEST(wait_ms_below_min);
	RUN_TEST(wait_ms_above_max);
	RUN_TEST(repeat_not_bool);
	RUN_TEST(reverse_not_bool);
	RUN_TEST(rotate_not_int);
	RUN_TEST(rotate_below_min);
	RUN_TEST(rotate_above_max);

	RUN_TEST(rotate_below_length);
	RUN_TEST(rotate_above_length);

	RUN_TEST(byte_array_multiple_of_3);
	RUN_TEST(rotate_with_unsupported_objects);
	RUN_TEST(reverse_with_unsupported_objects);
}
