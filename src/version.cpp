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

#include "aurcor/modaurcor.h"

#ifndef NO_QSTR
# include <Arduino.h>

extern "C" {
	# include <py/obj.h>
	# include <py/objstr.h>
	# include <py/objtuple.h>
}

# include <cstdint>
# include <limits>
#endif

namespace aurcor {

namespace micropython {

class ModuleVersionPreReleaseString;

class ModuleVersion {
	friend class ModuleVersionPreReleaseString;

public:
	constexpr ModuleVersion(const char *version) {
		const char *pos = version;
		bool valid = true;

		while (valid && pos[0] >= '0' && pos[0] <= '9') {
			major = (major * 10) + (pos[0] - '0');
			pos++;
		}

		if (pos[0] != '.') {
			valid = false;
		} else if (valid) {
			pos++;
		}

		while (valid && pos[0] >= '0' && pos[0] <= '9') {
			minor = (minor * 10) + (pos[0] - '0');
			pos++;
		}

		if (pos[0] != '.') {
			valid = false;
		} else if (valid) {
			pos++;
		}

		while (valid && pos[0] >= '0' && pos[0] <= '9') {
			patch = (patch * 10) + (pos[0] - '0');
			pos++;
		}

		if (!valid) {
			major = -1;
			minor = -1;
			patch = -1;
			pos = version;
		}

		if (pos[0] != '\0') {
			pre_release = pos;

			while (pos[0] != '\0') {
				pre_release_length++;
				pos++;
			}

			pre_release_length++;
		}
	}

	constexpr bool has_pre_release() const {
		return pre_release_length > 0;
	}

	long long major{0};
	long long minor{0};
	long long patch{0};

private:
	const char *pre_release{nullptr};
	size_t pre_release_length{0};
};

static constexpr ModuleVersion module_version{APP_VERSION};

class ModuleVersionPreReleaseString {
public:
	constexpr ModuleVersionPreReleaseString() {
		if (module_version.has_pre_release()) {
			const char *version = module_version.pre_release;

			for (size_t i = 0; version[0] != '\0'; i++, version++) {
				text[i] = version[0];
			}
		}
	}

	char text[module_version.pre_release_length] = {};
};

static constexpr ModuleVersionPreReleaseString module_version_pre_release_str{};

} // namespace micropython

} // namespace aurcor

using aurcor::micropython::module_version;
using aurcor::micropython::module_version_pre_release_str;

static inline constexpr uintptr_t mp_small_int_to_uintptr_obj_t(const long long value) {
	static_assert(MICROPY_OBJ_REPR == MICROPY_OBJ_REPR_C, "MicroPython must use object representation C");
	return (((uintptr_t)value << 1) | 1);
}

static_assert(module_version.major >= (std::numeric_limits<int>::min() >> 1), "Negative major value outside supported range");
static_assert(module_version.major <= (std::numeric_limits<int>::max() >> 1), "Positive major value outside supported range");
static_assert(module_version.minor >= (std::numeric_limits<int>::min() >> 1), "Negative minor value outside supported range");
static_assert(module_version.minor <= (std::numeric_limits<int>::max() >> 1), "Positive minor value outside supported range");
static_assert(module_version.patch >= (std::numeric_limits<int>::min() >> 1), "Negative patch value outside supported range");
static_assert(module_version.patch <= (std::numeric_limits<int>::max() >> 1), "Positive patch value outside supported range");

extern "C" {

static constexpr const MP_DEFINE_STR_OBJ(aurcor_version_pre_release_obj PROGMEM,
	module_version_pre_release_str.text);

constexpr const aurcor_version_t aurcor_version_obj PROGMEM = {
	{ &mp_type_tuple },
	module_version.has_pre_release() ? 4 : 3,
	{
		mp_small_int_to_uintptr_obj_t(module_version.major),
		mp_small_int_to_uintptr_obj_t(module_version.minor),
		mp_small_int_to_uintptr_obj_t(module_version.patch),
		module_version.has_pre_release()
			? (uintptr_t)&aurcor_version_pre_release_obj : 0,
	},
};

} // extern "C"

