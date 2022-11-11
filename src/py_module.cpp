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
#include <algorithm>

extern "C" {
	# include <py/binary.h>
	# include <py/runtime.h>
	# include <py/obj.h>
	# include <py/qstr.h>
}

# include "aurcor/led_profile.h"
# include "aurcor/led_profiles.h"
# include "aurcor/memory_pool.h"
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

PyModule::PyModule(MemoryBlock *led_buffer) : led_buffer_(led_buffer) {
}

inline PyModule& PyModule::current() {
	return aurcor::MicroPython::current().modaurcor_;
}

mp_obj_t PyModule::output_leds(OutputType type, size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	enum {
		ARG_values,
		ARG_length,
		ARG_profile,
		ARG_fps,
		ARG_wait_ms,
		ARG_repeat,
		ARG_reverse,
		ARG_rotate,
	};
	static constexpr size_t N_BEFORE_DEFAULTS = 2;
	static constexpr size_t N_AFTER_DEFAULTS = 1;
	static const mp_arg_t allowed_args[] = {
		// BEFORE_DEFAULTS
		{MP_QSTR_,            MP_ARG_REQUIRED | MP_ARG_OBJ,   {u_obj: MP_OBJ_NULL}},
		{MP_QSTR_,            MP_ARG_OBJ,                     {u_obj: MP_OBJ_NULL}},

		{MP_QSTR_profile,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_fps,         MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_ms,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_repeat,      MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_reverse,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},

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
	auto wait_us = set_defaults ? DEFAULT_WAIT_US : wait_us_;
	auto repeat = set_defaults ? DEFAULT_REPEAT : repeat_;
	auto reverse = set_defaults ? DEFAULT_REPEAT : reverse_;

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

		if (value != 0 && (value < MIN_FPS || value > MAX_FPS))
			mp_raise_ValueError(MP_ERROR_TEXT("fps out of range"));

		if (value == 0) {
			wait_us = 0;
		} else {
			wait_us = 1000000 / value;
		}
	}

	if (parsed_args[ARG_wait_ms].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(parsed_args[ARG_wait_ms].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("wait_ms must be an int"));

		mp_int_t value = mp_obj_get_int(parsed_args[ARG_wait_ms].u_obj);

		if (value != 0 && (value < MIN_WAIT_MS || value > MAX_WAIT_MS))
			mp_raise_ValueError(MP_ERROR_TEXT("wait_ms out of range"));

		wait_us = value * 1000;
	}

	if (parsed_args[ARG_repeat].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_bool(parsed_args[ARG_repeat].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("repeat must be a bool"));

		repeat = mp_obj_is_true(parsed_args[ARG_repeat].u_obj);
	}

	if (parsed_args[ARG_reverse].u_obj != MP_ROM_NONE) {
		if (!mp_obj_is_bool(parsed_args[ARG_reverse].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("reverse must be a bool"));

		reverse = mp_obj_is_true(parsed_args[ARG_reverse].u_obj);
	}

	if (type == OutputType::DEFAULTS) {
		profile_ = profile;
		wait_us_ = wait_us;
		repeat_ = repeat;
		reverse_ = reverse;
		return MP_ROM_NONE;
	}

	ssize_t rotate_length = parsed_args[ARG_rotate].u_int;
	auto values = parsed_args[ARG_values].u_obj;
	uint8_t *buffer = current().led_buffer_->begin();
	const size_t max_bytes = std::min(MAX_LEDS * BYTES_PER_LED, // TODO get current bus length
		current().led_buffer_->size());
	size_t in_bytes = max_bytes;
	size_t out_bytes = 0;

	mp_buffer_info_t bufinfo;
	bool byte_array = mp_get_buffer(values, &bufinfo, MP_BUFFER_READ) && is_byte_array(bufinfo);

	if (byte_array) {
		in_bytes = std::min(in_bytes, bufinfo.len);

		if (type == OutputType::HSL)
			mp_raise_ValueError(MP_ERROR_TEXT("can't use byte array for HSL values"));
	}

	if (parsed_args[ARG_length].u_obj != MP_OBJ_NULL) {
		if (!mp_obj_is_int(parsed_args[ARG_length].u_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("length must be an int"));

		mp_int_t value = mp_obj_get_int(parsed_args[ARG_length].u_obj);

		if (value < 0)
			mp_raise_TypeError(MP_ERROR_TEXT("length must be positive"));

		in_bytes = std::min(in_bytes, (size_t)value * BYTES_PER_LED);
	}

	if (byte_array) {
		const size_t buf_bytes = bufinfo.len;
		const size_t rotate_bytes = rotate_length > 0
			? ((size_t)rotate_length * BYTES_PER_LED)
			: (buf_bytes - (size_t)std::abs(rotate_length) * BYTES_PER_LED);

		if (rotate_bytes > buf_bytes)
			mp_raise_ValueError(MP_ERROR_TEXT("can't rotate by more than the length of byte array"));

		if (!reverse) {
			if (rotate_bytes != 0) {
				size_t available_bytes = std::min(in_bytes, buf_bytes - rotate_bytes);

				append_bytes(buffer, type, bufinfo, rotate_bytes, available_bytes, out_bytes);
				out_bytes += available_bytes;
				in_bytes -= available_bytes;
			}

			if (in_bytes > 0) {
				append_bytes(buffer, type, bufinfo, 0, in_bytes, out_bytes);
				out_bytes += in_bytes;
			}
		} else {
			if (rotate_bytes != 0) {
				size_t available_bytes = std::min(in_bytes, rotate_bytes);

				in_bytes -= available_bytes;
				for (size_t i = rotate_bytes; available_bytes > 0; i -= BYTES_PER_LED) {
					append_bytes(buffer, type, bufinfo, i, BYTES_PER_LED, out_bytes);
					available_bytes -= BYTES_PER_LED;
				}
			}

			for (size_t i = buf_bytes; in_bytes > 0; i -= BYTES_PER_LED) {
				append_bytes(buffer, type, bufinfo, i, BYTES_PER_LED, out_bytes);
				out_bytes += BYTES_PER_LED;
				in_bytes -= BYTES_PER_LED;
			}

		}
	} else if (rotate_length != 0 || reverse) {
		const size_t values_length = mp_obj_get_int(mp_obj_len(values));
		const size_t abs_rotate_length = rotate_length > 0
			? (size_t)rotate_length
			: (values_length - (size_t)std::abs(rotate_length));
		size_t in_length = std::min(in_bytes / BYTES_PER_LED, values_length);
		auto values_subscr = mp_obj_get_type(values)->subscr;

		if (values_subscr == nullptr) {
			mp_obj_subscr(values, 0, MP_OBJ_SENTINEL);
			mp_raise_TypeError(MP_ERROR_TEXT("values must be subscriptable"));
		}

		if (reverse) {
			if (rotate_length != 0) {
				size_t available_length = std::min(in_length, abs_rotate_length);

				in_length -= available_length;
				for (size_t i = abs_rotate_length; available_length > 0; i--) {
					append_led(buffer, type, values_subscr(values, MP_ROM_INT(i - 1), MP_OBJ_SENTINEL), out_bytes);
					out_bytes += BYTES_PER_LED;
					available_length--;
				}
			}

			for (size_t i = values_length; in_length > 0; i--) {
				append_led(buffer, type, values_subscr(values, MP_ROM_INT(i - 1), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		} else {
			size_t available_length = std::min(in_length, values_length - abs_rotate_length);

			for (size_t i = abs_rotate_length; available_length > 0; i++) {
				append_led(buffer, type, values_subscr(values, MP_ROM_INT(i), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				available_length--;
			}
			in_length -= available_length;

			for (size_t i = 0; in_length > 0; i++) {
				append_led(buffer, type, values_subscr(values, MP_ROM_INT(i), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		}
	} else {
		size_t in_length = in_bytes / BYTES_PER_LED;
		mp_obj_iter_buf_t iter_buf;
		mp_obj_t iterable = mp_getiter(values, &iter_buf);

		while (in_length > 0) {
			mp_obj_t item = mp_iternext(iterable);

			if (item == MP_OBJ_STOP_ITERATION)
				break;

			append_led(buffer, type, item, out_bytes);
			out_bytes += BYTES_PER_LED;
			in_length--;
		}
	}

	if (repeat && out_bytes > 0) {
		while (out_bytes < max_bytes) {
			size_t available_bytes = std::min(out_bytes, max_bytes - out_bytes);

			::memcpy(&buffer[out_bytes], &buffer[0], available_bytes);
			out_bytes += available_bytes;
		}
	}

	return MP_ROM_NONE;
}

inline void PyModule::append_bytes(uint8_t *buffer, OutputType type,
		const mp_buffer_info_t &bufinfo, size_t offset, size_t bytes,
		size_t out_bytes) {
	auto src = reinterpret_cast<uint8_t *>(bufinfo.buf);

	if (type == OutputType::RGB) {
		::memcpy(&buffer[out_bytes], &src[offset], bytes);
	} else { /* OutputType::HSL */
		while (bytes > 0) {
			hsl_to_rgb(src, offset, buffer, out_bytes);
			offset += BYTES_PER_LED;
			out_bytes += BYTES_PER_LED;
			bytes -= BYTES_PER_LED;
		}
	}
}

void PyModule::append_led(uint8_t *buffer, OutputType type, mp_obj_t item, size_t offset) {
	// TODO parse item
}

void PyModule::hsl_to_rgb(uint8_t *src, size_t src_offset, uint8_t *dst, size_t dst_offset) {
	// TODO convert HSL to RGB
}

} // namespace micropython

} // namespace aurcor
