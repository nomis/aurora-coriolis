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
	# include <py/runtime.h>
	# include <py/obj.h>
	# include <py/objstr.h>
	# include <py/objtuple.h>
	# include <py/qstr.h>
}

# include "aurcor/led_profile.h"
# include "aurcor/led_profiles.h"
# include "aurcor/micropython.h"
#endif

using aurcor::MicroPython;
using aurcor::micropython::PyModule;

extern "C" {

mp_obj_t aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::RGB, n_args, args, kwargs);
}

mp_obj_t aurcor_output_hsl(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::HSL, n_args, args, kwargs);
}

mp_obj_t aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::DEFAULTS, n_args, args, kwargs);
}

} // extern "C"

namespace aurcor {

namespace micropython {

inline PyModule& PyModule::current() {
	return aurcor::MicroPython::current().modaurcor_;
}

mp_obj_t PyModule::output_leds(OutputType type, size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	enum { ARG_values, ARG_profile, ARG_fps, ARG_wait_ms, ARG_repeat, ARG_rotate };
	static constexpr size_t N_BEFORE_DEFAULTS = 1;
	static constexpr size_t N_AFTER_DEFAULTS = 1;
	static const mp_arg_t allowed_args[] = {
		// BEFORE_DEFAULTS
		{MP_QSTR_,            MP_ARG_REQUIRED | MP_ARG_OBJ,   {u_obj: MP_OBJ_NULL}},

		{MP_QSTR_profile,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_fps,         MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_ms,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_repeat,      MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},

		// AFTER_DEFAULTS
		{MP_QSTR_rotate,      MP_ARG_KW_ONLY | MP_ARG_INT,    {u_int: 0}},
	};
	const bool set_defaults = type == OutputType::DEFAULTS;
	const size_t n_allowed_args = MP_ARRAY_SIZE(allowed_args)
		- (set_defaults ? (N_BEFORE_DEFAULTS + N_AFTER_DEFAULTS) : 0);
	const size_t off_allowed_args = set_defaults ? N_BEFORE_DEFAULTS : 0;
	mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all(n_args, args, kwargs, n_allowed_args,
		&allowed_args[off_allowed_args],
		&parsed_args[off_allowed_args]);

	auto profile = set_defaults ? DEFAULT_PROFILE : profile_;
	auto fps = set_defaults ? NO_FPS : fps_;
	auto wait_ms = set_defaults ? NO_WAIT_MS : wait_ms_;
	auto repeat = set_defaults ? DEFAULT_REPEAT : repeat_;

	if (parsed_args[ARG_profile].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(parsed_args[ARG_profile].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("profile must be an int"));

		mp_int_t value = mp_obj_get_int(parsed_args[ARG_profile].u_obj);

		if (value < (mp_int_t)LEDProfiles::MIN_ID
				|| value > (mp_int_t)LEDProfiles::MAX_ID)
			mp_raise_ValueError(MP_ERROR_TEXT("invalid profile"));

		profile = (enum led_profile_id)value;
	}

	if (parsed_args[ARG_fps].u_obj != MP_ROM_NONE
			&& parsed_args[ARG_wait_ms].u_obj != MP_ROM_NONE) {
		mp_raise_ValueError(MP_ERROR_TEXT("can't specify both fps and wait_ms at the same time"));
	}

	if (parsed_args[ARG_fps].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(parsed_args[ARG_fps].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("fps must be an int"));

		mp_int_t value = mp_obj_get_int(parsed_args[ARG_fps].u_obj);

		if (value != NO_FPS && (value < MIN_FPS || value > MAX_FPS))
			mp_raise_ValueError(MP_ERROR_TEXT("fps out of range"));

		fps = value;
		wait_ms = NO_WAIT_MS; // override default
	}

	if (parsed_args[ARG_wait_ms].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(parsed_args[ARG_wait_ms].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("wait_ms must be an int"));

		mp_int_t value = mp_obj_get_int(parsed_args[ARG_wait_ms].u_obj);

		if (value != NO_WAIT_MS && (value < MIN_WAIT_MS || value > MAX_WAIT_MS))
			mp_raise_ValueError(MP_ERROR_TEXT("wait_ms out of range"));

		wait_ms = value;
		fps = NO_FPS; // override default
	}

	if (parsed_args[ARG_repeat].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_bool(parsed_args[ARG_repeat].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("repeat must be a bool"));

		repeat = mp_obj_is_true(parsed_args[ARG_repeat].u_obj);
	}

	if (type == OutputType::DEFAULTS) {
		profile_ = profile;
		fps_ = fps;
		wait_ms_ = wait_ms;
		repeat_ = repeat;
		return MP_ROM_NONE;
	}

	// ssize_t rotate = parsed_args[ARG_rotate].u_int;

	return MP_ROM_NONE;
}

} // namespace micropython

} // namespace aurcor
