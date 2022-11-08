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
extern "C" {
	# include <py/obj.h>
	# include <py/objstr.h>
	# include <py/objtuple.h>
}
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

	mp_int_t major{0};
	mp_int_t minor{0};
	mp_int_t patch{0};

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

extern "C" {

static MP_DEFINE_STR_OBJ(aurcor_version_pre_release_obj,
	aurcor::micropython::module_version_pre_release_str.text);

const aurcor_version_t aurcor_version_obj = {
	{ &mp_type_tuple },
	aurcor::micropython::module_version.has_pre_release() ? 4 : 3,
	{
		MP_ROM_INT(aurcor::micropython::module_version.major),
		MP_ROM_INT(aurcor::micropython::module_version.minor),
		MP_ROM_INT(aurcor::micropython::module_version.patch),
		aurcor::micropython::module_version.has_pre_release()
			? MP_ROM_PTR(&aurcor_version_pre_release_obj) : 0,
	},
};

} // extern "C"

