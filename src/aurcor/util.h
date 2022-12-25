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

#pragma once

#ifndef ENV_NATIVE
# include <esp_timer.h>
#endif

#include <algorithm>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

extern "C" {
	#include <py/mpconfig.h>
	#include <py/objtuple.h>
}

#define TUPLE_FIXED0(tuple, alloc) \
	struct { \
		mp_obj_base_t base; \
		size_t len; \
		mp_obj_t items[alloc]; \
	} tuple = { &mp_type_tuple, 0, { 0 } }

namespace aurcor {

// These are in priority order to allow combining errors
enum Result : uint8_t {
	OK = 0,
	FULL,
	NOT_FOUND,
	OUT_OF_RANGE,
	PARSE_ERROR,
	IO_ERROR,
};

static inline Result downgrade_result(Result &current, Result result) {
	current = std::max(current, result);
	return result;
}

static inline uint64_t current_time_us() {
#ifdef ENV_NATIVE
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#else
	return esp_timer_get_time();
#endif
}

static inline int int_constrain(int value, int max, int min = 0) {
	return std::max(min, std::min(max, value));
}

static inline uint8_t int_to_u8(int value) {
	return int_constrain(value, UINT8_MAX);
}

/* Increase precision of integer division without using floating point */
static inline constexpr int int_divide(int divided, int divisor, unsigned int bits) {
	return ((divided << (bits + 1U)) / divisor + (1 << bits)) >> (bits + 1U);
}

static inline constexpr unsigned int uint_divide(unsigned int divided, unsigned int divisor, unsigned int bits) {
	return ((divided << (bits + 1U)) / divisor + (1U << bits)) >> (bits + 1U);
}

bool allowed_file_name(const std::string &name);
bool allowed_text(const std::string &text);
std::vector<std::string> list_filenames(const char *directory_name, const char *extension);

template <class T>
static constexpr size_t rounded_sizeof() {
	return (sizeof(T) + (alignof(T) - 1)) / alignof(T) * alignof(T);
}

namespace container {

template <class T, class V>
static inline void add(std::set<T> &container, const V &&value) {
	container.insert(value);
}

template <class T, class V>
static inline void add(std::vector<T> &container, const V &&value) {
	container.push_back(value);
}

} // namespace

} // namespace aurcor
