/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2023  Simon Arlott
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
# include <sys/time.h>

# include <algorithm>
# include <array>
# include <cassert>
# include <cmath>
# include <cstring>

extern "C" {
	# include <py/binary.h>
	# include <py/runtime.h>
	# include <py/obj.h>
	# include <py/objtuple.h>
	# include <py/qstr.h>
	# include <py/smallint.h>
	# include <extmod/utime_mphal.h>
	# include <shared/timeutils/timeutils.h>
}

# include "aurcor/led_profile.h"
# include "aurcor/led_profiles.h"
# include "aurcor/memory_pool.h"
# include "aurcor/micropython.h"
# include "aurcor/preset.h"
# include "aurcor/util.h"
#endif

#ifdef ENV_NATIVE
# define inline
#endif

using aurcor::MicroPython;
using aurcor::micropython::PyModule;

extern "C" {

mp_obj_t aurcor_length() {
	return PyModule::current().length();
}

mp_obj_t aurcor_default_fps() {
	return PyModule::current().default_fps();
}

mp_obj_t aurcor_register_config(mp_obj_t dict) {
	return PyModule::current().register_config(dict);
}

mp_obj_t aurcor_config(mp_obj_t dict) {
	return PyModule::current().config(dict);
}

mp_obj_t aurcor_next_ticks30_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_ticks30_ms(n_args, args, kwargs);
}

mp_obj_t aurcor_next_ticks64_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_ticks64_ms(n_args, args, kwargs);
}

mp_obj_t aurcor_next_ticks64_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_ticks64_us(n_args, args, kwargs);
}

mp_obj_t aurcor_next_time(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_time(n_args, args, kwargs);
}

mp_obj_t aurcor_next_time_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_time_ms(n_args, args, kwargs);
}

mp_obj_t aurcor_next_time_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().next_time_us(n_args, args, kwargs);
}

mp_obj_t aurcor_hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args) {
	PyModule::hsv_to_rgb_buffer(n_args, args, false);
	return MP_ROM_NONE;
}

mp_obj_t aurcor_exp_hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args) {
	PyModule::hsv_to_rgb_buffer(n_args, args, true);
	return MP_ROM_NONE;
}

mp_obj_t aurcor_hsv_to_rgb_int(size_t n_args, const mp_obj_t *args) {
	return PyModule::hsv_to_rgb_int(n_args, args, false);
}

mp_obj_t aurcor_exp_hsv_to_rgb_int(size_t n_args, const mp_obj_t *args) {
	return PyModule::hsv_to_rgb_int(n_args, args, true);
}

mp_obj_t aurcor_hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args) {
	return PyModule::hsv_to_rgb_tuple(n_args, args, false);
}

mp_obj_t aurcor_exp_hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args) {
	return PyModule::hsv_to_rgb_tuple(n_args, args, true);
}

mp_obj_t aurcor_rgb_to_hsv_tuple(size_t n_args, const mp_obj_t *args) {
	return PyModule::rgb_to_hsv_tuple(n_args, args, false);
}

mp_obj_t aurcor_rgb_to_exp_hsv_tuple(size_t n_args, const mp_obj_t *args) {
	return PyModule::rgb_to_hsv_tuple(n_args, args, true);
}

mp_obj_t aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(n_args, args, kwargs, PyModule::OutputType::RGB, false);
}

mp_obj_t aurcor_output_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(n_args, args, kwargs, PyModule::OutputType::HSV, false);
}

mp_obj_t aurcor_output_exp_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(n_args, args, kwargs, PyModule::OutputType::EXP_HSV, false);
}

mp_obj_t aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(n_args, args, kwargs, PyModule::OutputType::RGB, true);
}

} // extern "C"

namespace aurcor {

namespace micropython {

PyModule::PyModule(MemoryBlock *led_buffer, std::shared_ptr<LEDBus> bus,
		Preset &preset) : led_buffer_(led_buffer), bus_(std::move(bus)),
		bus_length_(bus_->length()), bus_default_fps_(bus_->default_fps()),
		preset_(preset) {
}

inline PyModule& PyModule::current() {
	return MicroPython::current().modaurcor_;
}

mp_obj_t PyModule::length() {
	return MP_OBJ_NEW_SMALL_INT(bus_length_);
}

mp_obj_t PyModule::default_fps() {
	return MP_OBJ_NEW_SMALL_INT(bus_default_fps_);
}

mp_obj_t PyModule::register_config(mp_obj_t dict) {
	preset_.register_config(dict);
	return MP_ROM_NONE;
}

mp_obj_t PyModule::config(mp_obj_t dict) {
	bool ret = preset_.populate_config(dict);
	size_t bus_length = bus_->length();
	unsigned int bus_default_fps = bus_->default_fps();

	if (bus_length != bus_length_) {
		bus_length_ = bus_length;
		ret = true;
	}

	if (bus_default_fps != bus_default_fps_) {
		bus_default_fps_ = bus_default_fps;
		ret = true;
	}

	if (!config_used_) {
		config_used_ = true;
		ret = true;
	}

	return mp_obj_new_bool(ret);
}

mp_obj_t PyModule::output_leds(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs, OutputType type, bool set_defaults) {
	enum {
		ARG_values,

		ARG_profile,
		ARG_fps,
		ARG_wait_ms,
		ARG_wait_us,
		ARG_repeat,
		ARG_reverse,

		ARG_rotate,
	};
	static constexpr size_t N_BEFORE_DEFAULTS = 1;
	static constexpr size_t N_AFTER_DEFAULTS = 1;
	static const mp_arg_t allowed_args[] = {
		// BEFORE_DEFAULTS
		{MP_QSTR_values,      MP_ARG_REQUIRED | MP_ARG_OBJ,   {u_obj: MP_OBJ_NULL}},

		{MP_QSTR_profile,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_fps,         MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_ms,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_us,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_repeat,      MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_reverse,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},

		// AFTER_DEFAULTS
		{MP_QSTR_rotate,      MP_ARG_KW_ONLY | MP_ARG_INT,    {u_int: 0}},
	};
	const size_t n_allowed_args = MP_ARRAY_SIZE(allowed_args)
		- (set_defaults ? (N_BEFORE_DEFAULTS + N_AFTER_DEFAULTS) : 0);
	const size_t off_allowed_args = set_defaults ? N_BEFORE_DEFAULTS : 0;
	mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all(n_args, args, kwargs, n_allowed_args,
		&allowed_args[off_allowed_args],
		&parsed_args[off_allowed_args]);

	auto profile = set_defaults ? DEFAULT_PROFILE : profile_;
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

	long wait_us = calc_wait_us(parsed_args[ARG_fps].u_obj,
		parsed_args[ARG_wait_ms].u_obj, parsed_args[ARG_wait_us].u_obj,
		set_defaults);

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

	if (set_defaults) {
		profile_ = profile;
		wait_us_ = wait_us;
		repeat_ = repeat;
		reverse_ = reverse;
		return MP_ROM_NONE;
	}

	ssize_t signed_rotate_length = parsed_args[ARG_rotate].u_int;
	auto values = parsed_args[ARG_values].u_obj;
	uint8_t *buffer = led_buffer_->begin();
	const size_t max_bytes = std::min(bus_length_ * BYTES_PER_LED, led_buffer_->size());
	size_t in_bytes = max_bytes;
	size_t out_bytes = 0;

	if (signed_rotate_length < MIN_SLENGTH || signed_rotate_length > MAX_SLENGTH)
		mp_raise_msg(&mp_type_OverflowError, MP_ERROR_TEXT("overflow converting rotate value to bytes"));

	mp_buffer_info_t bufinfo;
	bool byte_array = false;
	bool generator_reverse = false;

	// Prevent use of unspecified array types so that they can be repurposed in the future
	if (mp_get_buffer(values, &bufinfo, MP_BUFFER_READ)) {
		switch (bufinfo.typecode) {
		case BYTEARRAY_TYPECODE:
		case 'b':
		case 'B':
			byte_array = true;
			break;

		// 0-65535 Hue
		case 'h': // short
		case 'H': // unsigned short
		// 0.0 <= Hue < 1.0
		case 'f': // float
			if (type == OutputType::RGB)
				mp_raise_TypeError(MP_ERROR_TEXT("unsupported array type for RGB values"));
			break;

		// 0x__RRGGBB
		case 'i': // int
		case 'I': // unsigned int
			if (type != OutputType::RGB)
				mp_raise_TypeError(MP_ERROR_TEXT("unsupported array type for HSV values"));
			break;

		case 'O': // object
			break;

		case 'l': // long
		case 'L': // unsigned long
		case 'q': // long long
		case 'Q': // unsigned long long
		case 'd': // double
		case 'P': // pointer
		default:
			mp_raise_TypeError(MP_ERROR_TEXT("unsupported array type"));
			break;
		}
	} else if (mp_obj_len_maybe(values) == MP_OBJ_NULL) {
		if (reverse && signed_rotate_length == 0) {
			reverse = false;
			generator_reverse = true;
		}
	}

	if (byte_array) {
		const size_t buf_bytes = bufinfo.len;

		if (type != OutputType::RGB)
			mp_raise_TypeError(MP_ERROR_TEXT("can only use byte array for RGB values"));

		if (buf_bytes % BYTES_PER_LED != 0)
			mp_raise_ValueError(MP_ERROR_TEXT("byte array length must be a multiple of 3 bytes"));

		if ((size_t)std::abs(signed_rotate_length) * BYTES_PER_LED > buf_bytes)
			mp_raise_ValueError(MP_ERROR_TEXT("can't rotate by more than the length of byte array"));

		in_bytes = std::min(in_bytes, bufinfo.len);

		const uint8_t *input = reinterpret_cast<uint8_t *>(bufinfo.buf);
		const size_t rotate_bytes = signed_rotate_length >= 0
			? (signed_rotate_length * BYTES_PER_LED)
			: (buf_bytes + signed_rotate_length * BYTES_PER_LED);
		size_t available_rotate_bytes = std::min(in_bytes, buf_bytes - rotate_bytes);

		if (reverse) {
			in_bytes -= available_rotate_bytes;
			for (size_t i = buf_bytes - rotate_bytes - BYTES_PER_LED; available_rotate_bytes > 0; i -= BYTES_PER_LED) {
				std::memcpy(&buffer[out_bytes], &input[i], BYTES_PER_LED);
				out_bytes += BYTES_PER_LED;
				available_rotate_bytes -= BYTES_PER_LED;
			}

			for (size_t i = buf_bytes - BYTES_PER_LED; in_bytes > 0; i -= BYTES_PER_LED) {
				std::memcpy(&buffer[out_bytes], &input[i], BYTES_PER_LED);
				out_bytes += BYTES_PER_LED;
				in_bytes -= BYTES_PER_LED;
			}
		} else {
			if (available_rotate_bytes > 0) {
				std::memcpy(&buffer[out_bytes], &input[rotate_bytes], available_rotate_bytes);
				out_bytes += available_rotate_bytes;
				in_bytes -= available_rotate_bytes;
			}

			if (in_bytes > 0) {
				std::memcpy(&buffer[out_bytes], &input[0], in_bytes);
				out_bytes += in_bytes;
				in_bytes = 0;
			}
		}
	} else if (signed_rotate_length != 0 || reverse) {
		const size_t values_length = mp_obj_get_int(mp_obj_len(values));

		if ((size_t)std::abs(signed_rotate_length) > values_length)
			mp_raise_ValueError(MP_ERROR_TEXT("can't rotate by more than the length of values"));

		const size_t rotate_length = signed_rotate_length >= 0
			? signed_rotate_length : (values_length + signed_rotate_length);
		size_t in_length = std::min(in_bytes / BYTES_PER_LED, values_length);
		size_t available_rotate_length = std::min(in_length, values_length - rotate_length);

		if (reverse) {
			in_length -= available_rotate_length;
			for (size_t i = values_length - rotate_length - 1; available_rotate_length > 0; i--) {
				append_led(type, buffer, out_bytes, mp_obj_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));
				out_bytes += BYTES_PER_LED;
				available_rotate_length--;
			}

			for (size_t i = values_length - 1; in_length > 0; i--) {
				append_led(type, buffer, out_bytes, mp_obj_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		} else {
			in_length -= available_rotate_length;
			for (size_t i = rotate_length; available_rotate_length > 0; i++) {
				append_led(type, buffer, out_bytes, mp_obj_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));
				out_bytes += BYTES_PER_LED;
				available_rotate_length--;
			}

			for (size_t i = 0; in_length > 0; i++) {
				append_led(type, buffer, out_bytes, mp_obj_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		}
	} else {
		size_t in_length = in_bytes / BYTES_PER_LED;
		mp_obj_iter_buf_t iter_buf;
		mp_obj_t iterable = mp_getiter(values, &iter_buf);

		if (generator_reverse) {
			out_bytes = max_bytes;

			while (in_length > 0) {
				mp_obj_t item = mp_iternext(iterable);

				if (item == MP_OBJ_STOP_ITERATION)
					break;

				out_bytes -= BYTES_PER_LED;
				append_led(type, buffer, out_bytes, item);
				in_length--;
			}
		} else {
			while (in_length > 0) {
				mp_obj_t item = mp_iternext(iterable);

				if (item == MP_OBJ_STOP_ITERATION)
					break;

				append_led(type, buffer, out_bytes, item);
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		}
	}

	if (repeat && out_bytes > 0 && out_bytes < max_bytes) {
		if (generator_reverse) {
			do {
				size_t available_bytes = std::min(out_bytes, max_bytes - out_bytes);
				assert(available_bytes > 0);

				std::memcpy(&buffer[out_bytes - available_bytes], &buffer[max_bytes - available_bytes], available_bytes);
				out_bytes -= available_bytes;
			} while (out_bytes > 0);
		} else {
			do {
				size_t available_bytes = std::min(out_bytes, max_bytes - out_bytes);

				std::memcpy(&buffer[out_bytes], &buffer[0], available_bytes);
				out_bytes += available_bytes;
			} while (out_bytes < max_bytes);
		}
	}

	if (generator_reverse) {
		if (out_bytes > 0)
			std::memset(buffer, 0, out_bytes);

		out_bytes = max_bytes;
	}

	bus_->profile(profile).transform(buffer, out_bytes);

	if (wait_us > 0 && bus_written_) {
		uint64_t start_us = bus_->last_update_us() + wait_us - TIMING_DELAY_US;
		uint64_t now_us = current_time_us();

		if (start_us > now_us)
			mp_hal_delay_us(start_us - now_us);
	}

	bus_->write(buffer, out_bytes, preset_.reverse());
	bus_written_ = true;

	if (!config_used_) {
		bus_length_ = bus_->length();
		bus_default_fps_ = bus_->default_fps();
	}

	return MP_ROM_NONE;
}

void PyModule::append_led(OutputType type, uint8_t *buffer, size_t offset, mp_obj_t item) {
	if (mp_obj_is_int(item) || mp_obj_is_float(item)) {
		switch (type) {
		case OutputType::RGB: {
				mp_int_t value = mp_obj_get_int(item); /* RGB doesn't support float */

				offset += 2;
				for (size_t i = 0; i < 3; i++) {
					buffer[offset--] = value;
					value >>= 8;
				}
				break;
			}

		case OutputType::HSV:
			hsv_to_rgb(hue_obj_to_int(item, false), MAX_SATURATION, MAX_VALUE, &buffer[offset]);
			break;

		case OutputType::EXP_HSV:
			exp_hsv_to_rgb(hue_obj_to_int(item, true), MAX_SATURATION, MAX_VALUE, &buffer[offset]);
			break;
		}
	} else {
		switch (type) {
		case OutputType::RGB:
			for (size_t i = 0; i < 3; i++)
				buffer[offset++] = int_to_u8(mp_obj_get_int(mp_obj_subscr(item, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL)));
			break;

		case OutputType::HSV:
		case OutputType::EXP_HSV:
			mp_int_t len = int_constrain(mp_obj_get_int(mp_obj_len(item)), 3);
			mp_obj_t hue_obj = mp_obj_subscr(item, MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_SENTINEL);
			mp_int_t saturation = MAX_SATURATION;
			mp_int_t value = MAX_VALUE;

			switch (len) {
			case 3:
				saturation = saturation_obj_to_int(mp_obj_subscr(item, MP_OBJ_NEW_SMALL_INT(1), MP_OBJ_SENTINEL));
				[[fallthrough]];
			case 2:
				value = value_obj_to_int(mp_obj_subscr(item, MP_OBJ_NEW_SMALL_INT(len - 1), MP_OBJ_SENTINEL));
				break;
			}

			if (type == OutputType::HSV) {
				hsv_to_rgb(hue_obj_to_int(hue_obj, false), saturation, value, &buffer[offset]);
			} else {
				exp_hsv_to_rgb(hue_obj_to_int(hue_obj, true), saturation, value, &buffer[offset]);
			}

			break;
		}
	}
}

void PyModule::hsv_to_rgb(mp_int_t hue, mp_int_t saturation, mp_int_t value, uint8_t rgb[3]) {
	if (HSV_TO_RGB_USE_FLOAT) {
		mp_float_t hi;
		mp_float_t hf = std::modf(hue / (mp_float_t)(HUE_RANGE / 6), &hi);
		mp_float_t s = saturation / (mp_float_t)MAX_SATURATION;
		mp_float_t v = value * (UINT8_MAX / (mp_float_t)MAX_VALUE);
		int_fast8_t k = (hue / (HUE_RANGE / 6)) % 6;
		int_fast8_t q = k >> 1;
		int_fast8_t p = (0b010010 >> (q << 1)) & 0b11;
		int_fast8_t t = (0b001001 >> (q << 1)) & 0b11;

		rgb[p] = int_to_u8(std::lround(v * (1 - s)));
		rgb[q] = int_to_u8(std::lround((k & 1) ? v * (1 - s * hf) : v));
		rgb[t] = int_to_u8(std::lround((k & 1) ? v : v * (1 - s * (1 - hf))));
	} else {
		static constexpr const int HF_PRECISION = 1000;
		static constexpr const int V_PRECISION = 32;
		uint_fast32_t hf = uint_divide((hue % (HUE_RANGE / 6)) * HF_PRECISION, HUE_RANGE / 6, 1);
		uint_fast32_t vp = uint_divide(value * UINT8_MAX * V_PRECISION, MAX_VALUE, 1);
		uint8_t v = uint_divide(vp, V_PRECISION, 1);
		int_fast8_t k = (hue / (HUE_RANGE / 6)) % 6;
		int_fast8_t q = k >> 1;
		int_fast8_t p = (0b010010 >> (q << 1)) & 0b11;
		int_fast8_t t = (0b001001 >> (q << 1)) & 0b11;

		rgb[p] = uint_divide(vp * (MAX_SATURATION - saturation), V_PRECISION * MAX_SATURATION, 1);
		if (k & 1) {
			rgb[t] = v;
			rgb[q] = uint_divide(vp * ((MAX_SATURATION * HF_PRECISION) - (saturation * hf)),
				V_PRECISION * MAX_SATURATION * HF_PRECISION, 1);
		} else {
			rgb[q] = v;
			rgb[t] = uint_divide(vp * ((MAX_SATURATION * HF_PRECISION) - (saturation * (HF_PRECISION - hf))),
				V_PRECISION * MAX_SATURATION * HF_PRECISION, 1);
		}
	}
}

inline void PyModule::exp_hsv_to_rgb(mp_int_t expanded_hue, mp_int_t saturation, mp_int_t value, uint8_t rgb[3]) {
	mp_int_t hue;

	if (expanded_hue < EXPANDED_HUE_LEFT_RANGE) {
		hue = expanded_hue / EXPANDED_HUE_TIMES;
	} else {
		hue = expanded_hue - EXPANDED_HUE_RIGHT_OFFSET;
	}

	hsv_to_rgb(hue, saturation, value, rgb);
}

void PyModule::hsv_to_rgb(size_t n_args, const mp_obj_t *args, bool exp, uint8_t rgb[3]) {
	mp_int_t hue = hue_obj_to_int(args[0], exp);
	mp_int_t saturation = MAX_SATURATION;
	mp_int_t value = MAX_VALUE;

	switch (n_args) {
	case 3:
		saturation = saturation_obj_to_int(args[1]);
		[[fallthrough]];
	case 2:
		value = value_obj_to_int(args[n_args - 1]);
		break;
	}

	if (exp) {
		exp_hsv_to_rgb(hue, saturation, value, rgb);
	} else {
		hsv_to_rgb(hue, saturation, value, rgb);
	}
}

void PyModule::hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args, bool exp) {
	enum { ARG_buffer, ARG_offset };
	mp_buffer_info_t bufinfo;
	size_t offset;

	mp_get_buffer_raise(args[ARG_buffer], &bufinfo, MP_BUFFER_WRITE);

	switch (bufinfo.typecode) {
	case BYTEARRAY_TYPECODE:
	case 'b':
	case 'B':
		break;

	default:
		mp_raise_TypeError(MP_ERROR_TEXT("buffer must be a byte array"));
		break;
	}

	if (bufinfo.len % BYTES_PER_LED != 0)
		mp_raise_TypeError(MP_ERROR_TEXT("byte array length must be a multiple of 3 bytes"));

	if (!mp_obj_is_int(args[ARG_offset])) {
			mp_raise_TypeError(MP_ERROR_TEXT("buffer index must be an int"));
	} else {
		mp_int_t value = mp_obj_get_int(args[ARG_offset]);

		if (value < 0)
			mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index must be positive"));

		if ((size_t)value > MAX_ULENGTH)
			mp_raise_msg(&mp_type_OverflowError, MP_ERROR_TEXT("overflow converting buffer index to bytes"));

		offset = (size_t)value * BYTES_PER_LED;

		if (offset > bufinfo.len)
			mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));

		if (bufinfo.len - offset < BYTES_PER_LED)
			mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));
	}

	hsv_to_rgb(n_args - 2, &args[2], exp, &reinterpret_cast<uint8_t*>(bufinfo.buf)[offset]);
}

mp_obj_t PyModule::hsv_to_rgb_int(size_t n_args, const mp_obj_t *args, bool exp) {
	std::array<uint8_t,3> rgb_bytes;

	hsv_to_rgb(n_args, args, exp, rgb_bytes.data());

	static_assert(MP_SMALL_INT_FITS(0xFFFFFF), "small int overflow");
	return MP_OBJ_NEW_SMALL_INT((rgb_bytes[0] << 16) | (rgb_bytes[1] << 8) | rgb_bytes[2]);
}

mp_obj_t PyModule::hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args, bool exp) {
	std::array<uint8_t,3> rgb_bytes;
	mp_obj_tuple_t *tuple = reinterpret_cast<mp_obj_tuple_t *>(mp_obj_new_tuple(rgb_bytes.size(), nullptr));

	hsv_to_rgb(n_args, args, exp, rgb_bytes.data());

	for (size_t i = 0; i < rgb_bytes.size(); i++)
		tuple->items[i] = MP_OBJ_NEW_SMALL_INT(rgb_bytes[i]);

	return tuple;
}

void PyModule::rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, mp_int_t hsv[3]) {
	enum { HSV_hue, HSV_saturation, HSV_value };
	uint8_t max = r;

	if (g > max) max = g;
	if (b > max) max = b;

	if (max == 0) {
		hsv[HSV_value] = 0;
		hsv[HSV_saturation] = 0;
		hsv[HSV_hue] = 0;
		return;
	}

	if (RGB_TO_HSV_USE_FLOAT) {
		hsv[HSV_value] = std::lround(max * ((mp_float_t)MAX_VALUE / (mp_float_t)UINT8_MAX));
	} else {
		hsv[HSV_value] = uint_divide(max * MAX_VALUE, UINT8_MAX, 1);
	}

	uint8_t min = r;

	if (g < min) min = g;
	if (b < min) min = b;

	uint8_t c = max - min;

	if (c == 0) {
		hsv[HSV_saturation] = 0;
		hsv[HSV_hue] = 0;
		return;
	}

	if (RGB_TO_HSV_USE_FLOAT) {
		hsv[HSV_saturation] = std::lround((c * MAX_VALUE) / (mp_float_t)max);
	} else {
		hsv[HSV_saturation] = uint_divide(c * MAX_VALUE, max, 1);
	}

	int_fast32_t h1;
	int_fast32_t h2;

	if (r == max) {
		h1 = (b == min) ? 0 : HUE_RANGE;
		h2 = g - b;
	} else if (g == max) {
		h1 = HUE_RANGE / 3;
		h2 = b - r;
	} else {
		h1 = HUE_RANGE / 3 * 2;
		h2 = r - g;
	}

	if (RGB_TO_HSV_USE_FLOAT) {
		hsv[HSV_hue] = std::lround(h1 + ((HUE_RANGE / 6) * h2) / (mp_float_t)c);
	} else {
		hsv[HSV_hue] = h1 + int_divide((HUE_RANGE / 6) * h2, c, 8);
	}
}

inline void PyModule::rgb_to_exp_hsv(uint8_t r, uint8_t g, uint8_t b, mp_int_t hsv[3]) {
	rgb_to_hsv(r, g, b, hsv);

	if (hsv[0] < EXPANDED_HUE_SIZE) {
		hsv[0] *= EXPANDED_HUE_TIMES;
	} else {
		hsv[0] += EXPANDED_HUE_RIGHT_OFFSET;
	}
}

mp_obj_t PyModule::rgb_to_hsv_tuple(size_t n_args, const mp_obj_t *args, bool exp) {
	uint8_t r, g, b;
	std::array<mp_int_t,3> hsv;
	mp_obj_tuple_t *tuple = reinterpret_cast<mp_obj_tuple_t *>(mp_obj_new_tuple(hsv.size(), nullptr));

	if (n_args == 1) {
		mp_int_t value = mp_obj_get_int(args[0]);
		r = (value >> 16) & 0xFF;
		g = (value >> 8) & 0xFF;
		b = value & 0xFF;
	} else if (n_args == 3) {
		r = int_to_u8(mp_obj_get_int(args[0]));
		g = int_to_u8(mp_obj_get_int(args[1]));
		b = int_to_u8(mp_obj_get_int(args[2]));
	} else {
		mp_raise_TypeError(MP_ERROR_TEXT("must provide 1 combined int or 3 separate r/g/b ints"));
	}

	if (exp) {
		PyModule::rgb_to_exp_hsv(r, g, b, hsv.data());
	} else {
		PyModule::rgb_to_hsv(r, g, b, hsv.data());
	}

	for (size_t i = 0; i < hsv.size(); i++)
		tuple->items[i] = MP_OBJ_NEW_SMALL_INT(hsv[i]);

	return tuple;
}

inline mp_int_t PyModule::hue_obj_to_int(mp_obj_t hue, bool exp) {
	if (mp_obj_is_int(hue)) {
		return mp_obj_get_int(hue) % (exp ? EXPANDED_HUE_RANGE : HUE_RANGE);
	} else if (mp_obj_is_float(hue)) {
		mp_float_t h = mp_obj_get_float(hue);

		if (!std::isfinite(h))
			mp_raise_TypeError(MP_ERROR_TEXT("hue float must be finite"));

		mp_float_t hf = std::modf(h, &h);

		if (std::signbit(hf))
			hf = std::modf(hf + (mp_float_t)1.0, &h);

		return std::lround(hf * (exp ? EXPANDED_HUE_RANGE : HUE_RANGE));
	} else {
		mp_raise_TypeError(MP_ERROR_TEXT("hue must be an int or float"));
	}
}

inline mp_int_t PyModule::saturation_obj_to_int(mp_obj_t saturation) {
	if (mp_obj_is_int(saturation)) {
		return mp_obj_get_int(saturation);
	} else if (mp_obj_is_float(saturation)) {
		mp_float_t s = mp_obj_get_float(saturation);

		if (!std::isfinite(s))
			mp_raise_TypeError(MP_ERROR_TEXT("saturation float must be finite"));

		return int_constrain(std::lround(s * MAX_SATURATION), MAX_SATURATION);
	} else {
		mp_raise_TypeError(MP_ERROR_TEXT("saturation must be an int or float"));
	}
}

inline mp_int_t PyModule::value_obj_to_int(mp_obj_t value) {
	if (mp_obj_is_int(value)) {
		return mp_obj_get_int(value);
	} else if (mp_obj_is_float(value)) {
		mp_float_t v = mp_obj_get_float(value);

		if (!std::isfinite(v))
			mp_raise_TypeError(MP_ERROR_TEXT("value float must be finite"));

		return int_constrain(std::lround(v * MAX_VALUE), MAX_VALUE);
	} else {
		mp_raise_TypeError(MP_ERROR_TEXT("value must be an int or float"));
	}
}

long PyModule::calc_wait_us(mp_obj_t fps_obj, mp_obj_t wait_ms_obj, mp_obj_t wait_us_obj, bool set_defaults) {
	auto wait_us = set_defaults ? DEFAULT_WAIT_US : wait_us_;
	unsigned int set = 0;

	if (fps_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(fps_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("fps must be an int"));

		mp_int_t value = mp_obj_get_int(fps_obj);

		if (value < MIN_FPS || value > MAX_FPS)
			mp_raise_ValueError(MP_ERROR_TEXT("fps out of range"));

		wait_us = 1000000 / value;
		set++;
	}

	if (wait_ms_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(wait_ms_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("wait_ms must be an int"));

		mp_int_t value = mp_obj_get_int(wait_ms_obj);

		if (value < 0 || value > MAX_WAIT_MS)
			mp_raise_ValueError(MP_ERROR_TEXT("wait_ms out of range"));

		wait_us = value * 1000;
		set++;
	}

	if (wait_us_obj != MP_ROM_NONE) {
		if (!mp_obj_is_int(wait_us_obj))
			mp_raise_TypeError(MP_ERROR_TEXT("wait_us must be an int"));

		mp_int_t value = mp_obj_get_int(wait_us_obj);

		if (value < 0 || value > MAX_WAIT_US)
			mp_raise_ValueError(MP_ERROR_TEXT("wait_us out of range"));

		wait_us = value;
		set++;
	}

	if (set > 1)
		mp_raise_ValueError(MP_ERROR_TEXT("can't specify more than one of fps, wait_ms or wait_us at the same time"));

	if (!set_defaults && wait_us == DEFAULT_WAIT_US && bus_default_fps_ > 0)
		wait_us = 1000000 / bus_default_fps_;

	return wait_us;
}

void PyModule::next_wait_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs,
		uint64_t &now_us, uint64_t &start_us) {
	enum {
		ARG_fps,
		ARG_wait_ms,
		ARG_wait_us,
	};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_fps,         MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_ms,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
		{MP_QSTR_wait_us,     MP_ARG_KW_ONLY | MP_ARG_OBJ,    {u_obj: MP_ROM_NONE}},
	};
	mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all(n_args, args, kwargs, MP_ARRAY_SIZE(allowed_args),
		allowed_args, parsed_args);

	long wait_us = calc_wait_us(parsed_args[ARG_fps].u_obj,
		parsed_args[ARG_wait_ms].u_obj, parsed_args[ARG_wait_us].u_obj, false);

	if (wait_us && bus_written_) {
		start_us = bus_->last_update_us() + wait_us - TIMING_DELAY_US;
		now_us = current_time_us();

		if (now_us > start_us)
			start_us = now_us;
	} else {
		start_us = now_us = current_time_us();
	}
}

void PyModule::next_timeofday(struct timeval &tv, uint64_t offset_us) {
    gettimeofday(&tv, NULL);

	while (offset_us >= 1000000ULL) {
		tv.tv_sec++;
		offset_us -= 1000000ULL;
	}

	tv.tv_usec += offset_us;
	if (tv.tv_usec > 10000000) {
		tv.tv_sec++;
		tv.tv_usec -= 1000000;
	}
}

mp_obj_t PyModule::next_ticks30_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;

	next_wait_us(n_args, args, kwargs, now_us, start_us);

	return MP_OBJ_NEW_SMALL_INT((start_us / 1000ULL) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1));
}

mp_obj_t PyModule::next_ticks64_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;

	next_wait_us(n_args, args, kwargs, now_us, start_us);

	return mp_obj_new_int_from_ll(start_us / 1000ULL);
}

mp_obj_t PyModule::next_ticks64_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;

	next_wait_us(n_args, args, kwargs, now_us, start_us);

	return mp_obj_new_int_from_ll(start_us);
}

mp_obj_t PyModule::next_time(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;
	struct timeval tv;

	next_wait_us(n_args, args, kwargs, now_us, start_us);
	next_timeofday(tv, start_us - now_us);

	int32_t seconds = tv.tv_sec;
#if !MICROPY_EPOCH_IS_1970
	seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
#endif
    return mp_obj_new_int(seconds);
}

mp_obj_t PyModule::next_time_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;
	struct timeval tv;

	next_wait_us(n_args, args, kwargs, now_us, start_us);
	next_timeofday(tv, start_us - now_us);

	int32_t seconds = tv.tv_sec;
#if !MICROPY_EPOCH_IS_1970
	seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
#endif
	int64_t ms = seconds * 1000LL;
	ms += tv.tv_usec / 1000LL;
    return mp_obj_new_int_from_ll(ms);
}

mp_obj_t PyModule::next_time_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	uint64_t now_us;
	uint64_t start_us;
	struct timeval tv;

	next_wait_us(n_args, args, kwargs, now_us, start_us);
	next_timeofday(tv, start_us - now_us);

	int32_t seconds = tv.tv_sec;
#if !MICROPY_EPOCH_IS_1970
	seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
#endif
	int64_t us = seconds * 1000000LL;
	us += tv.tv_usec;
	return mp_obj_new_int_from_ll(us);
}


} // namespace micropython

} // namespace aurcor
