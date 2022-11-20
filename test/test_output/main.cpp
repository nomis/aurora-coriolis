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

#include "test_micropython.h"

void tearDown(void) {
	TestMicroPython::tearDown();
}

int main(int argc, char *argv[]) {
	int ret = 0;

	TestMicroPython::init();

#define FUNC_DECLARE_EXEC(func) \
	do { \
		extern int func(); \
		ret |= func(); \
	} while(0)

	FUNC_DECLARE_EXEC(testsuite_bytearray_rgb);
	FUNC_DECLARE_EXEC(testsuite_generator_int_rgb);
	FUNC_DECLARE_EXEC(testsuite_generator_list_rgb);
	FUNC_DECLARE_EXEC(testsuite_generator_tuple_rgb);
	FUNC_DECLARE_EXEC(testsuite_intarray_rgb);
	FUNC_DECLARE_EXEC(testsuite_list_int_rgb);
	FUNC_DECLARE_EXEC(testsuite_list_list_rgb);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_rgb);
	FUNC_DECLARE_EXEC(testsuite_objarray_int_rgb);
	FUNC_DECLARE_EXEC(testsuite_objarray_list_rgb);
	FUNC_DECLARE_EXEC(testsuite_objarray_tuple_rgb);
	FUNC_DECLARE_EXEC(testsuite_tuple_int_rgb);
	FUNC_DECLARE_EXEC(testsuite_tuple_list_rgb);
	FUNC_DECLARE_EXEC(testsuite_tuple_tuple_rgb);

	FUNC_DECLARE_EXEC(testsuite_generator_list_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_generator_list_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_generator_tuple_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_generator_tuple_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hsv_float_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hsv_float_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_objarray_tuple_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_objarray_tuple_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_tuple_tuple_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_tuple_tuple_hsv_int);

	FUNC_DECLARE_EXEC(testsuite_generator_list_exp_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_generator_list_exp_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_generator_tuple_exp_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_generator_tuple_exp_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hsv_float_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hsv_float_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_objarray_tuple_exp_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_objarray_tuple_exp_hsv_int);
	FUNC_DECLARE_EXEC(testsuite_tuple_tuple_exp_hsv_float);
	FUNC_DECLARE_EXEC(testsuite_tuple_tuple_exp_hsv_int);

	FUNC_DECLARE_EXEC(testsuite_list_tuple_hv_float);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hv_float_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hv_float_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_hv_int);

	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hv_float);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hv_float_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hv_float_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_tuple_exp_hv_int);

	FUNC_DECLARE_EXEC(testsuite_floatarray_h);
	FUNC_DECLARE_EXEC(testsuite_list_float_h);
	FUNC_DECLARE_EXEC(testsuite_list_float_h_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_float_h_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_int_h);
	FUNC_DECLARE_EXEC(testsuite_shortarray_h);

	FUNC_DECLARE_EXEC(testsuite_floatarray_exp_h);
	FUNC_DECLARE_EXEC(testsuite_list_float_exp_h);
	FUNC_DECLARE_EXEC(testsuite_list_float_exp_h_neg1);
	FUNC_DECLARE_EXEC(testsuite_list_float_exp_h_pos1);
	FUNC_DECLARE_EXEC(testsuite_list_int_exp_h);
	FUNC_DECLARE_EXEC(testsuite_shortarray_exp_h);

	FUNC_DECLARE_EXEC(testsuite_invalid);

	return ret;
}
