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

#include "aurcor/modaurcor.h"

using aurcor::micropython::PyModule;

static constexpr mp_int_t RED = 0;
static constexpr mp_int_t YELLOW = 60;
static constexpr mp_int_t GREEN = 120;
static constexpr mp_int_t CYAN = 180;
static constexpr mp_int_t BLUE = 240;
static constexpr mp_int_t MAGENTA = 300;

static constexpr mp_int_t EXP_RED = RED;
static constexpr mp_int_t EXP_YELLOW = YELLOW + 60;
static constexpr mp_int_t EXP_GREEN = GREEN + 60;
static constexpr mp_int_t EXP_CYAN = CYAN + 60;
static constexpr mp_int_t EXP_BLUE = BLUE + 60;
static constexpr mp_int_t EXP_MAGENTA = MAGENTA + 60;

static void test_standard_hues_to_rgb_and_back() {
	std::array<uint8_t,3> rgb;
	std::array<mp_int_t,3> hsv;

	PyModule::hsv_to_rgb(RED, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 0, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(30, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 128, 0, hsv);
	TEST_ASSERT_EQUAL_INT(30, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(YELLOW, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(255, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(60, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(90, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(128, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(90, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(GREEN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(120, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(150, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 128, hsv);
	TEST_ASSERT_EQUAL_INT(150, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(CYAN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(180, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(210, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 128, 255, hsv);
	TEST_ASSERT_EQUAL_INT(210, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(BLUE, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(0, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(240, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(270, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(128, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(270, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(MAGENTA, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(300, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::hsv_to_rgb(330, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(255, 0, 128, hsv);
	TEST_ASSERT_EQUAL_INT(330, hsv[0]);
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
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(30, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(64, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 64, 0, hsv);
	TEST_ASSERT_EQUAL_INT(30, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(60, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 128, 0, hsv);
	TEST_ASSERT_EQUAL_INT(60, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(90, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(191, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 191, 0, hsv);
	TEST_ASSERT_EQUAL_INT(90, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_YELLOW, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(120, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(150, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(128, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(150, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_GREEN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(0, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 0, hsv);
	TEST_ASSERT_EQUAL_INT(180, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(210, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 128, hsv);
	TEST_ASSERT_EQUAL_INT(210, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_CYAN, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 255, 255, hsv);
	TEST_ASSERT_EQUAL_INT(240, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(270, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 128, 255, hsv);
	TEST_ASSERT_EQUAL_INT(270, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_BLUE, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(0, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(0, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(300, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(330, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(128, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(330, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(EXP_MAGENTA, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 0, 255, hsv);
	TEST_ASSERT_EQUAL_INT(360, hsv[0]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);

	PyModule::exp_hsv_to_rgb(390, PyModule::MAX_SATURATION, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(0, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_exp_hsv(255, 0, 128, hsv);
	TEST_ASSERT_EQUAL_INT(390, hsv[0]);
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

	PyModule::hsv_to_rgb(90, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::hsv_to_rgb(180, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::hsv_to_rgb(270, 0, PyModule::MAX_VALUE, rgb);
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

	PyModule::exp_hsv_to_rgb(90, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::exp_hsv_to_rgb(180, 0, PyModule::MAX_VALUE, rgb);
	TEST_ASSERT_EQUAL_INT(255, rgb[0]);
	TEST_ASSERT_EQUAL_INT(255, rgb[1]);
	TEST_ASSERT_EQUAL_INT(255, rgb[2]);

	PyModule::exp_hsv_to_rgb(270, 0, PyModule::MAX_VALUE, rgb);
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

	PyModule::hsv_to_rgb(90, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::hsv_to_rgb(180, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::hsv_to_rgb(270, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::rgb_to_hsv(128, 128, 128, hsv);
	TEST_ASSERT_EQUAL_INT(0, hsv[0]);
	TEST_ASSERT_EQUAL_INT(0, hsv[1]);
	TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE / 2, hsv[2]);

	PyModule::exp_hsv_to_rgb(0, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(90, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(180, 0, PyModule::MAX_VALUE / 2, rgb);
	TEST_ASSERT_EQUAL_INT(128, rgb[0]);
	TEST_ASSERT_EQUAL_INT(128, rgb[1]);
	TEST_ASSERT_EQUAL_INT(128, rgb[2]);

	PyModule::exp_hsv_to_rgb(270, 0, PyModule::MAX_VALUE / 2, rgb);
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
	for (size_t v = 0; v < PyModule::MAX_VALUE; v++) {
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

	for (size_t v = 0; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(RED, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::exp_hsv_to_rgb(RED, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_exp_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}
}

static void test_red_values() {
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(RED, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(RED, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(YELLOW, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
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
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(YELLOW, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[1]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(YELLOW, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(GREEN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
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
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(GREEN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(GREEN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(CYAN, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
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
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(CYAN, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(rgb[1], rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(CYAN, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(BLUE, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
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
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(BLUE, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[0]);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(BLUE, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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
	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(MAGENTA, s, PyModule::MAX_VALUE, rgb);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(s, hsv[1]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_VALUE, hsv[2]);
	}

	for (size_t s = 1; s < PyModule::MAX_SATURATION; s++) {
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
	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
		std::array<uint8_t,3> rgb;
		std::array<mp_int_t,3> hsv;

		PyModule::hsv_to_rgb(MAGENTA, PyModule::MAX_SATURATION, v, rgb);
		TEST_ASSERT_EQUAL_INT(0, rgb[1]);
		TEST_ASSERT_EQUAL_INT(rgb[0], rgb[2]);

		PyModule::rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv);
		TEST_ASSERT_EQUAL_INT(MAGENTA, hsv[0]);
		TEST_ASSERT_EQUAL_INT(PyModule::MAX_SATURATION, hsv[1]);
		TEST_ASSERT_EQUAL_INT(v, hsv[2]);
	}

	for (size_t v = 1; v < PyModule::MAX_VALUE; v++) {
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

	return UNITY_END();
}
