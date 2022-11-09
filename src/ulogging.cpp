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
# include <algorithm>

# include <cstring>

extern "C" {
	# include <py/runtime.h>
	# include <py/obj.h>
	# include <py/objstr.h>
	# include <py/qstr.h>
}

# include <uuid/log.h>

# include "aurcor/micropython.h"
#endif

using aurcor::MicroPython;
using aurcor::micropython::ULogging;
using uuid::log::Level;
using uuid::log::Logger;

extern "C" {

mp_obj_t ulogging_log(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return ULogging::do_log(MP_QSTR_log, ULogging::level_from_obj(args[0]), false, n_args - 1, &args[1], kwargs);
}

#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
	mp_obj_t ulogging_ ## _py_lc_name (size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) { \
		return ULogging::do_log(MP_QSTR_ ## _py_lc_name, ULOGGING_L_ ## _py_uc_name, false, n_args, args, kwargs); \
	}

ULOGGING_LEVELS
#undef ULOGGING_LEVEL

mp_obj_t ulogging_exception(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return ULogging::do_log(MP_QSTR_exception, ULOGGING_L_ERROR, true, n_args, args, kwargs);
}

mp_obj_t ulogging_disable(size_t n_args, const mp_obj_t *args) {
	mp_int_t level = ULOGGING_L_OFF;

	if (n_args >= 1)
		level = ULogging::level_from_obj(args[0]);

	ULogging::disable(level);
	return MP_ROM_NONE;
}

mp_obj_t ulogging_getEffectiveLevel() {
	return MP_OBJ_NEW_SMALL_INT(ULogging::effective_level());
}

mp_obj_t ulogging_isEnabledFor(mp_obj_t level_o) {
	return ULogging::enabled_level(ULogging::level_from_obj(level_o)) != Level::OFF ? MP_ROM_TRUE : MP_ROM_FALSE;
}

mp_obj_t ulogging_setLevel(mp_obj_t level) {
	ULogging::enable(ULogging::level_from_obj(level));
	return MP_ROM_NONE;
}

} // extern "C"

namespace aurcor {

namespace micropython {

inline ULogging& ULogging::current() {
	return MicroPython::current().modulogging_;
}

mp_int_t ULogging::level_from_obj(mp_obj_t level_o) {
	if (!mp_obj_is_int(level_o))
		mp_raise_TypeError(MP_ERROR_TEXT("level must be an int"));

	return mp_obj_get_int(level_o);
}

inline Level ULogging::find_level(mp_int_t py_level) {
#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
		if (py_level >= ULOGGING_L_ ## _py_uc_name) return _level;
	ULOGGING_LEVELS

#undef ULOGGING_LEVEL

	return Level::TRACE;
}

inline mp_int_t ULogging::to_py_level(Level level) {
	switch (level) {
#define ULOGGING_LEVEL(_level, _py_lc_name, _py_uc_name, _py_level) \
		case _level: return _py_level;

ULOGGING_LEVELS_WITH_META
#undef ULOGGING_LEVEL
	}

	return ULOGGING_L_NOTSET;
}

mp_obj_t ULogging::do_log(qstr fn, mp_int_t py_level, bool exc_info,
		size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	static constexpr size_t STACK_TUPLE_NUM = 8;
	auto &self = current();
	Level level = self.enabled_level(py_level);

	if (level == Level::OFF)
		return MP_ROM_NONE;

	enum { ARG_exc_info };
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_exc_info, MP_ARG_KW_ONLY | MP_ARG_OBJ, {u_obj: MP_OBJ_NULL}},
	};
	mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all(0, nullptr, kwargs, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

	auto print = MicroPython::current().modulogging_print(level);

	if (n_args == 1) {
		mp_obj_print_helper(print->context(), args[0], PRINT_STR);
	} else {
		mp_obj_t message;

		if (n_args == 2 && mp_obj_is_type(args[1], &mp_type_dict)) {
			message = mp_obj_str_binary_op(MP_BINARY_OP_MODULO, args[0], args[1]);
		} else if (n_args - 1 <= STACK_TUPLE_NUM) {
			TUPLE_FIXED0(tuple, STACK_TUPLE_NUM);

			tuple.len = n_args - 1;
			std::memcpy(&tuple.items[0], &args[1], tuple.len * sizeof(tuple.items[0]));

			message = mp_obj_str_binary_op(MP_BINARY_OP_MODULO, args[0], MP_OBJ_FROM_PTR(&tuple));
		} else {
			message = mp_obj_str_binary_op(MP_BINARY_OP_MODULO, args[0],
				mp_obj_new_tuple(n_args - 1, &args[1]));
		}

		mp_obj_print_helper(print->context(), message, PRINT_STR);
	}

	if (parsed_args[ARG_exc_info].u_obj != MP_OBJ_NULL)
		exc_info = mp_obj_is_true(parsed_args[ARG_exc_info].u_obj);

	if (exc_info) {
		mp_obj_t exc = MP_ROM_NONE;

		if (parsed_args[ARG_exc_info].u_obj != MP_OBJ_NULL
				&& mp_obj_is_exception_instance(parsed_args[ARG_exc_info].u_obj)) {
			exc = parsed_args[ARG_exc_info].u_obj;
#if MICROPY_PY_SYS_EXC_INFO
		} else if (MP_OBJ_FROM_PTR(MP_STATE_VM(cur_exception)) != MP_OBJ_NULL) {
			exc = MP_OBJ_FROM_PTR(MP_STATE_VM(cur_exception));
#endif
		}

		print = MicroPython::current().modulogging_print(level);

		mp_obj_print_exception(print->context(), exc);
	}

	return MP_ROM_NONE;
}

Level ULogging::enabled_level(mp_int_t py_level) {
	auto &self = current();

	if (py_level <= self.disable_level_)
		return Level::OFF;

	if (py_level < self.enable_level_)
		return Level::OFF;

	Level level = find_level(py_level);

	if (level > MicroPython::current().modulogging_effective_level())
		level = Level::OFF;

	return level;
}

mp_int_t ULogging::effective_level() {
	Level level = MicroPython::current().modulogging_effective_level();

	return std::max(current().enable_level_, to_py_level(level));
}

inline void ULogging::enable(mp_int_t py_level) {
	current().enable_level_ = py_level;
}

inline void ULogging::disable(mp_int_t py_level) {
	current().disable_level_ = py_level;
}

} // namespace micropython

} // namespace aurcor
