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

#include "test_colours.h"

#include <array>
#include <cmath>
#include <cstdint>

#include "aurcor/modaurcor.h"
#include "aurcor/util.h"

using aurcor::int_to_u8;
using aurcor::micropython::PyModule;

void fp_hsv_to_rgb(int hue360, int saturation, int value, std::array<uint8_t,3> &rgb) {
	float hi;
	float hf = std::modf(hue360 / (float)(PyModule::HUE_RANGE / 6), &hi);
	float s = saturation / (float)PyModule::MAX_SATURATION;
	float v = value * ((float)UINT8_MAX / (float)PyModule::MAX_VALUE);
	int_fast8_t k = (hue360 / (PyModule::HUE_RANGE / 6)) % 6;
	int_fast8_t q = k >> 1;
	int_fast8_t p = (0b010010 >> (q << 1)) & 0b11;
	int_fast8_t t = (0b001001 >> (q << 1)) & 0b11;

	rgb[p] = int_to_u8(std::lround(v * (1 - s)));
	rgb[q] = int_to_u8(std::lround((k & 1) ? v * (1 - s * hf) : v));
	rgb[t] = int_to_u8(std::lround((k & 1) ? v : v * (1 - s * (1 - hf))));
}

void fp_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, std::array<int,3> &hsv) {
	enum { HSV_hue, HSV_saturation, HSV_value };
	uint8_t max = r;

	if (g > max) max = g;
	if (b > max) max = b;

	if (max == 0) {
		hsv[HSV_value] = 0;
		hsv[HSV_saturation] = 0;
		hsv[HSV_hue] = 0;
		return;
	}

	hsv[HSV_value] = std::lround(max * ((float)PyModule::MAX_VALUE / (float)UINT8_MAX));

	uint8_t min = r;

	if (g < min) min = g;
	if (b < min) min = b;

	uint8_t c = max - min;

	if (c == 0) {
		hsv[HSV_saturation] = 0;
		hsv[HSV_hue] = 0;
		return;
	}

	hsv[HSV_saturation] = std::lround((c * PyModule::MAX_VALUE) / (float)max);

	int h1;
	int h2;

	if (r == max) {
		h1 = (b == min) ? 0 : PyModule::HUE_RANGE;
		h2 = g - b;
	} else if (g == max) {
		h1 = PyModule::HUE_RANGE / 3;
		h2 = b - r;
	} else {
		h1 = PyModule::HUE_RANGE / 3 * 2;
		h2 = r - g;
	}

	hsv[HSV_hue] = std::lround(h1 + ((PyModule::HUE_RANGE / 6) * h2) / (float)c);
}
