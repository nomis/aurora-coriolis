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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_QSTR
# include <py/runtime.h>
# include <py/obj.h>
# include <py/qstr.h>
#endif

// #define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level)
#define ULOGGING_LEVELS \
	ULOGGING_LEVEL(Level::EMERG,    emerg,    EMERG,    70) \
	ULOGGING_LEVEL(Level::ALERT,    alert,    ALERT,    60) \
	ULOGGING_LEVEL(Level::CRIT,     critical, CRITICAL, 50) \
	ULOGGING_LEVEL(Level::ERR,      error,    ERROR,    40) \
	ULOGGING_LEVEL(Level::WARNING,  warning,  WARNING,  30) \
	ULOGGING_LEVEL(Level::NOTICE,   notice,   NOTICE,   25) \
	ULOGGING_LEVEL(Level::INFO,     info,     INFO,     20) \
	ULOGGING_LEVEL(Level::DEBUG,    debug,    DEBUG,    10) \
	ULOGGING_LEVEL(Level::TRACE,    trace,    TRACE,     5)

#define ULOGGING_LEVELS_WITH_META \
	ULOGGING_LEVEL(Level::OFF,      off,      OFF,   10000) \
	ULOGGING_LEVELS \
	ULOGGING_LEVEL(Level::ALL,      all,      NOTSET,    0)

enum ulogging_py_levels {
#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
	ULOGGING_L_ ## _py_uc_name = _py_level,

ULOGGING_LEVELS_WITH_META
#undef ULOGGING_LEVEL
};

mp_obj_t ulogging_log(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(ulogging_log_obj);

#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
	mp_obj_t ulogging_ ## _py_lc_name (size_t n_args, const mp_obj_t *args, mp_map_t *kwargs); \
	MP_DECLARE_CONST_FUN_OBJ_KW(ulogging_ ## _py_lc_name ## _obj);

ULOGGING_LEVELS
#undef ULOGGING_LEVEL

mp_obj_t ulogging_exception(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(ulogging_exception_obj);

mp_obj_t ulogging_disable(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(ulogging_disable_obj);

mp_obj_t ulogging_getEffectiveLevel();
MP_DECLARE_CONST_FUN_OBJ_0(ulogging_getEffectiveLevel_obj);

mp_obj_t ulogging_isEnabledFor(mp_obj_t level);
MP_DECLARE_CONST_FUN_OBJ_1(ulogging_isEnabledFor_obj);

mp_obj_t ulogging_setLevel(mp_obj_t level);
MP_DECLARE_CONST_FUN_OBJ_1(ulogging_setLevel_obj);

#ifdef __cplusplus
} // extern "C"

#ifndef NO_QSTR
# include <uuid/log.h>
#endif

namespace aurcor {

namespace micropython {

class ULogging {
public:
	static mp_int_t level_from_obj(mp_obj_t level_o);
	static uuid::log::Level find_level(mp_int_t py_level);
	static mp_int_t to_py_level(uuid::log::Level level);

	static mp_obj_t do_log(qstr fn, mp_int_t py_level, bool exc_info,
		size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	static uuid::log::Level enabled_level(mp_int_t py_level);

	static mp_int_t effective_level();
	static void enable(mp_int_t py_level);
	static void disable(mp_int_t py_level);

private:
	static ULogging& current();

	mp_int_t enable_level_{ULOGGING_L_NOTSET};
	mp_int_t disable_level_{ULOGGING_L_NOTSET};
};

} // namespace micropython

} // namespace aurcor
#endif
