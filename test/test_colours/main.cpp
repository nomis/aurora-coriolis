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

#include <algorithm>
#include <array>
#include <cstdio>

#include "aurcor/modaurcor.h"
#include "test_colours.h"

using aurcor::micropython::PyModule;

static constexpr mp_int_t HUE_RANGE_MULTIPLIER = PyModule::HUE_RANGE / 360;

static constexpr mp_int_t RED = 0;
static constexpr mp_int_t YELLOW = 60;
static constexpr mp_int_t GREEN = 120;
static constexpr mp_int_t CYAN = 180;
static constexpr mp_int_t BLUE = 240;
static constexpr mp_int_t MAGENTA = 300;

static constexpr mp_int_t exp_multiplier(mp_int_t hue) {
	if (hue < 120) {
		return (hue / 2) * HUE_RANGE_MULTIPLIER * 2;
	} else {
		return (hue - 60) * HUE_RANGE_MULTIPLIER + 60 * HUE_RANGE_MULTIPLIER;
	}
}

static constexpr mp_int_t EXP_RED = exp_multiplier(RED);
static constexpr mp_int_t EXP_YELLOW = exp_multiplier(YELLOW + 60);
static constexpr mp_int_t EXP_GREEN = exp_multiplier(GREEN + 60);
static constexpr mp_int_t EXP_CYAN = exp_multiplier(CYAN + 60);
static constexpr mp_int_t EXP_BLUE = exp_multiplier(BLUE + 60);
static constexpr mp_int_t EXP_MAGENTA = exp_multiplier(MAGENTA + 60);

static void test_standard_hues_to_rgb_and_back() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * RED, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 0, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 30, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 128, 0, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 30, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * YELLOW, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 60, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 90, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(128, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 90, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * GREEN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 120, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 150, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 128, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 150, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * CYAN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 180, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 210, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 128, 255, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 210, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * BLUE, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 240, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 270, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(128, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 270, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * MAGENTA, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 300, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 330, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 128, hsv);
	TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * 330, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
}

static void test_expanded_hues_to_rgb_and_back() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::exp_hsv_to_rgb(EXP_RED, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 0, 0, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(0), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	// PyModule::exp_hsv_to_rgb(exp_multiplier(30), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	// TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	// TEST_ASSERT_EQUAL_INT(64, rgb[1]);
	// TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	// PyModule::rgb_to_exp_hsv(255, 64, 0, hsv);
	// TEST_ASSERT_EQUAL_INT(exp_multiplier(30), hsv[0]);
	// TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	// TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	// PyModule::exp_hsv_to_rgb(exp_multiplier(60), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	// TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	// TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	// TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	// PyModule::rgb_to_exp_hsv(255, 128, 0, hsv);
	// TEST_ASSERT_EQUAL_INT(exp_multiplier(60), hsv[0]);
	// TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	// TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(90), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(191, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 191, 0, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(90), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_YELLOW, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(120), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(150), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(128, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(150), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_GREEN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(180), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(210), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 128, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(210), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_CYAN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(240), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(270), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 128, 255, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(270), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_BLUE, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(300), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(330), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(128, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(330), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_MAGENTA, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(360), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(exp_multiplier(390), PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 0, 128, hsv);
	TEST_ASSERT_EQUAL_INT(exp_multiplier(390), hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
}

static void test_white() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::hsv_to_rgb(0, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 90, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 180, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 270, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(255, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(0, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 90, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 180, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 270, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
}

static void test_grey() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::hsv_to_rgb(0, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 90, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 180, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * 270, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(128, 128, 128, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE / 2, hsv[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 0, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 90, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 180, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(HUE_RANGE_MULTIPLIER * 270, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_exp_hsv(128, 128, 128, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE / 2, hsv[2]);
}

static void test_black() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::hsv_to_rgb((mp_int_t)0, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::hsv_to_rgb((mp_int_t)90, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::hsv_to_rgb((mp_int_t)180, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::hsv_to_rgb((mp_int_t)270, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(0, 0, 0, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(0, hsv[2]);

	PyModule::exp_hsv_to_rgb((mp_int_t)0, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::exp_hsv_to_rgb((mp_int_t)90, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::exp_hsv_to_rgb((mp_int_t)180, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::exp_hsv_to_rgb((mp_int_t)270, 0, 0, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 0, 0, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(0, hsv[2]);
}

static void test_greys() {
	for (size_t v = 0; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb((mp_int_t)0, 0, v, rgb);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[1]);
		TEST_ASSERT_EQUAL_INT(rgb[1], rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(0, hsv[0]);
		TEST_ASSERT_EQUAL_INT(0, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 0; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb((mp_int_t)0, 0, v, rgb);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[1]);
		TEST_ASSERT_EQUAL_INT(rgb[1], rgb[2]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(0, hsv[0]);
		TEST_ASSERT_EQUAL_INT(0, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_red_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * RED, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_RED, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_red_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * RED, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_RED, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_yellow_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * YELLOW, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_YELLOW, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_yellow_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * YELLOW, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[1]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_YELLOW, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[1]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_green_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * GREEN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_GREEN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_green_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * GREEN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_GREEN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_cyan_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * CYAN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_CYAN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_cyan_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * CYAN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(rgb[1], rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_CYAN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(rgb[1], rgb[2]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_blue_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * BLUE, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_BLUE, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_blue_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * BLUE, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_BLUE, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

static void test_magenta_saturation() {
	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * MAGENTA, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s <= PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_MAGENTA, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_magenta_values() {
	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(HUE_RANGE_MULTIPLIER * MAGENTA, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(HUE_RANGE_MULTIPLIER * MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v <= PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(EXP_MAGENTA, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[2]);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(EXP_MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}
}

/* Check integer calculations are as good as floating point calculations */
static void test_fp_hsv_to_rgb() {
	unsigned int exact1 = 0;
	unsigned int exact3 = 0;
	unsigned int diff1 = 0;
	unsigned int diff3 = 0;

	for (int h = 0; h < PyModule::HUE_RANGE; h++) {
		for (int s = 0; s <= PyModule::MAX_SATURATION; s++) {
			for (int v = 0; v <= PyModule::MAX_VALUE; v++) {
				std::array<uint8_t,3> rgb1;
				std::array<uint8_t,3> rgb2;

				PyModule::hsv_to_rgb(h, s, v, rgb1);
				fp_hsv_to_rgb(h, s, v, rgb2);

				int max_diff = std::max({
					std::abs((int)rgb1[0] - (int)rgb2[0]),
					std::abs((int)rgb1[1] - (int)rgb2[1]),
					std::abs((int)rgb1[2] - (int)rgb2[2])});

				if (max_diff > 1) {
					TEST_PRINTF("hsv %u %u %u -> rgb %u %u %u", h, s, v, rgb1[0], rgb1[1], rgb1[2]);
					TEST_PRINTF("hsv %u %u %u -> rgb %u %u %u (fp)", h, s, v, rgb2[0], rgb2[1], rgb2[2]);
					TEST_FAIL();
				} else if (max_diff > 0) {
					if (rgb1[0] != rgb2[0])
						diff1++;
					if (rgb1[1] != rgb2[1])
						diff1++;
					if (rgb1[2] != rgb2[2])
						diff1++;
					diff3++;
				} else {
					exact1 += 3;
					exact3++;
				}
			}
		}
	}

	float percent1 = (float)diff1 / (diff1 + exact1) * 100;
	TEST_PRINTF("value: total = %u exact = %u diff = %u (%g%%)", exact1 + diff1, exact1, diff1, percent1);
	TEST_ASSERT(percent1 < 1.0f);

	float percent3 = (float)diff3 / (diff3 + exact3) * 100;
	TEST_PRINTF("rgb: total = %u exact = %u diff = %u (%g%%)", exact3 + diff3, exact3, diff3, percent3);
	TEST_ASSERT(percent3 < 3.0f);
}

/* Check integer calculations are as good as floating point calculations */
static void test_fp_rgb_to_hsv() {
	for (int r = 0; r < UINT8_MAX; r++) {
		for (int g = 0; g < UINT8_MAX; g++) {
			for (int b = 0; b < UINT8_MAX; b++) {
				std::array<mp_int_t,3> hsv1;
				std::array<int,3> hsv2;

				PyModule::rgb_to_hsv(r, g, b, hsv1);
				fp_rgb_to_hsv(r, g, b, hsv2);

				if ((int)hsv1[0] != hsv2[0] || (int)hsv1[1] != hsv2[1] || (int)hsv1[2] != hsv2[2]) {
					TEST_PRINTF("rgb %u %u %u -> hsv %u %u %u", r, g, b, (int)hsv1[0], (int)hsv1[1], (int)hsv1[2]);
					TEST_PRINTF("rgb %u %u %u -> hsv %u %u %u (fp)", r, g, b, hsv2[0], hsv2[1], hsv2[2]);
					TEST_FAIL();
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();
	RUN_TEST(test_standard_hues_to_rgb_and_back);
	RUN_TEST(test_expanded_hues_to_rgb_and_back);

	RUN_TEST(test_white);
	RUN_TEST(test_grey);
	RUN_TEST(test_greys);
	RUN_TEST(test_black);

	RUN_TEST(test_red_saturation);
	RUN_TEST(test_red_values);
	RUN_TEST(test_yellow_saturation);
	RUN_TEST(test_yellow_values);
	RUN_TEST(test_green_saturation);
	RUN_TEST(test_green_values);
	RUN_TEST(test_cyan_saturation);
	RUN_TEST(test_cyan_values);
	RUN_TEST(test_blue_saturation);
	RUN_TEST(test_blue_values);
	RUN_TEST(test_magenta_saturation);
	RUN_TEST(test_magenta_values);

	RUN_TEST(test_fp_hsv_to_rgb);
	RUN_TEST(test_fp_rgb_to_hsv);

	return UNITY_END();
}
