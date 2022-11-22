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

#include <string>

static std::string output_fn_name = "output_hsv";

#define EXPECT_A 255,191,0
#define EXPECT_B 128,255,0
#define EXPECT_C 0,255,64
#define EXPECT_D 0,255,255
#define EXPECT_E 0,64,255
#define EXPECT_F 128,0,255
#define EXPECT_G 255,0,191

#include "common.ipp"
