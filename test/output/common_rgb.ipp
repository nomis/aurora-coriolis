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

static std::string output_fn_name = "output_rgb";

#define EXPECT_A 1,2,3
#define EXPECT_B 4,5,6
#define EXPECT_C 7,8,9
#define EXPECT_D 10,11,12
#define EXPECT_E 13,14,15
#define EXPECT_F 16,17,18
#define EXPECT_G 19,20,21

#include "common.ipp"
