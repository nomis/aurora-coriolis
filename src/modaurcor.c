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
# include <py/runtime.h>
# include <py/obj.h>
# include <py/qstr.h>
# include <extmod/utime_mphal.h>
#endif

#include "aurcor/constants.h"
#include "aurcor/led_profiles.h"

STATIC const mp_rom_map_elem_t aurcor_profiles_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__),          MP_ROM_QSTR(MP_QSTR_aurcor_dot_profiles) },

#define LED_PROFILE(_lc_name, _uc_name) \
	{ MP_ROM_QSTR(MP_QSTR_ ## _uc_name),      MP_ROM_INT(LED_PROFILE_ ## _uc_name) },
LED_PROFILES
#undef LED_PROFILE
};

STATIC MP_DEFINE_CONST_DICT(aurcor_profiles_module_globals, aurcor_profiles_module_globals_table);

const mp_obj_module_t aurcor_profiles_module = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&aurcor_profiles_module_globals,
};

/*
 * This doesn't work properly as of MicroPython v1.19.1
 * (the built-in module names aren't supposed to have dots in them).
 */
// MP_REGISTER_MODULE(MP_QSTR_aurcor_dot_profiles, aurcor_profiles_module);

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_buffer_obj, 3, 5, aurcor_hsv_to_rgb_buffer);
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_int_obj, 1, 3, aurcor_hsv_to_rgb_int);
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_tuple_obj, 1, 3, aurcor_hsv_to_rgb_tuple);
MP_DEFINE_CONST_FUN_OBJ_KW(aurcor_output_rgb_obj, 1, aurcor_output_rgb);
MP_DEFINE_CONST_FUN_OBJ_KW(aurcor_output_hsv_obj, 1, aurcor_output_hsv);
MP_DEFINE_CONST_FUN_OBJ_KW(aurcor_output_defaults_obj, 0, aurcor_output_defaults);

STATIC const mp_rom_map_elem_t aurcor_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__),          MP_ROM_QSTR(MP_QSTR_aurcor) },

	{ MP_ROM_QSTR(MP_QSTR_MAX_LEDS),          MP_ROM_INT(AURCOR_MAX_LEDS) },

	{ MP_ROM_QSTR(MP_QSTR_version),           MP_ROM_PTR(&aurcor_version_obj) },
	{ MP_ROM_QSTR(MP_QSTR_profiles),          MP_ROM_PTR(&aurcor_profiles_module) },

	{ MP_ROM_QSTR(MP_QSTR_ticks_ms),          MP_ROM_PTR(&mp_utime_ticks_ms_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ticks_us),          MP_ROM_PTR(&mp_utime_ticks_us_obj) },
	{ MP_ROM_QSTR(MP_QSTR_time),              MP_ROM_PTR(&mp_utime_time_obj) },
	{ MP_ROM_QSTR(MP_QSTR_timens),            MP_ROM_PTR(&mp_utime_time_ns_obj) },

	{ MP_ROM_QSTR(MP_QSTR_hsv_to_rgb_buffer), MP_ROM_PTR(&aurcor_hsv_to_rgb_buffer_obj) },
	{ MP_ROM_QSTR(MP_QSTR_hsv_to_rgb_int),    MP_ROM_PTR(&aurcor_hsv_to_rgb_int_obj) },
	{ MP_ROM_QSTR(MP_QSTR_hsv_to_rgb_tuple),  MP_ROM_PTR(&aurcor_hsv_to_rgb_tuple_obj) },

	{ MP_ROM_QSTR(MP_QSTR_output_rgb),        MP_ROM_PTR(&aurcor_output_rgb_obj) },
	{ MP_ROM_QSTR(MP_QSTR_output_hsv),        MP_ROM_PTR(&aurcor_output_hsv_obj) },
	{ MP_ROM_QSTR(MP_QSTR_output_defaults),   MP_ROM_PTR(&aurcor_output_defaults_obj) },
};

STATIC MP_DEFINE_CONST_DICT(aurcor_module_globals, aurcor_module_globals_table);

const mp_obj_module_t aurcor_module = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&aurcor_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_aurcor, aurcor_module);
