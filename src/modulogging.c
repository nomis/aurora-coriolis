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

#include "aurcor/modulogging.h"

#ifndef NO_QSTR
# include <py/runtime.h>
# include <py/obj.h>
# include <py/qstr.h>
#endif

#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
	MP_DEFINE_CONST_FUN_OBJ_KW(ulogging_ ## _py_lc_name ## _obj, 1, ulogging_ ## _py_lc_name);

ULOGGING_LEVELS
#undef ULOGGING_LEVEL

MP_DEFINE_CONST_FUN_OBJ_KW(ulogging_log_obj, 2, ulogging_log);
MP_DEFINE_CONST_FUN_OBJ_KW(ulogging_exception_obj, 1, ulogging_exception);
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ulogging_disable_obj, 0, 1, ulogging_disable);
MP_DEFINE_CONST_FUN_OBJ_0(ulogging_getEffectiveLevel_obj, ulogging_getEffectiveLevel);
MP_DEFINE_CONST_FUN_OBJ_1(ulogging_isEnabledFor_obj, ulogging_isEnabledFor);
MP_DEFINE_CONST_FUN_OBJ_1(ulogging_setLevel_obj, ulogging_setLevel);

STATIC const mp_rom_map_elem_t ulogging_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__),          MP_ROM_QSTR(MP_QSTR_ulogging) },

	{ MP_ROM_QSTR(MP_QSTR_NOTSET),            MP_ROM_INT(ULOGGING_L_NOTSET) },

#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
	{ MP_ROM_QSTR(MP_QSTR_ ## _py_uc_name),   MP_ROM_INT(ULOGGING_L_ ## _py_uc_name) }, \
	{ MP_ROM_QSTR(MP_QSTR_ ## _py_lc_name),   MP_ROM_PTR(&ulogging_ ## _py_lc_name ## _obj) },

ULOGGING_LEVELS
#undef ULOGGING_LEVEL

	{ MP_ROM_QSTR(MP_QSTR_log),               MP_ROM_PTR(&ulogging_log_obj) },
	{ MP_ROM_QSTR(MP_QSTR_exception),         MP_ROM_PTR(&ulogging_exception_obj) },

	{ MP_ROM_QSTR(MP_QSTR_disable),           MP_ROM_PTR(&ulogging_disable_obj) },
	{ MP_ROM_QSTR(MP_QSTR_getEffectiveLevel), MP_ROM_PTR(&ulogging_getEffectiveLevel_obj) },
	{ MP_ROM_QSTR(MP_QSTR_isEnabledFor),      MP_ROM_PTR(&ulogging_isEnabledFor_obj) },
	{ MP_ROM_QSTR(MP_QSTR_setLevel),          MP_ROM_PTR(&ulogging_setLevel_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ulogging_module_globals, ulogging_module_globals_table);

const mp_obj_module_t ulogging_module = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&ulogging_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ulogging, ulogging_module);
