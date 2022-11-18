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
# include <algorithm>
# include <array>
# include <cmath>
# include <cstring>

extern "C" {
	# include <py/binary.h>
	# include <py/runtime.h>
	# include <py/obj.h>
	# include <py/qstr.h>
	# include <py/smallint.h>
}

# include "aurcor/led_profile.h"
# include "aurcor/led_profiles.h"
# include "aurcor/memory_pool.h"
# include "aurcor/micropython.h"
# include "aurcor/util.h"
#endif

#ifdef ENV_NATIVE
# define inline
#endif

using aurcor::MicroPython;
using aurcor::micropython::PyModule;

extern "C" {

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
	return PyModule::current().output_leds(PyModule::OutputType::RGB, n_args, args, kwargs);
}

mp_obj_t aurcor_output_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::HSV, n_args, args, kwargs);
}

mp_obj_t aurcor_output_exp_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::EXP_HSV, n_args, args, kwargs);
}

mp_obj_t aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::DEFAULTS, n_args, args, kwargs);
}

} // extern "C"

namespace aurcor {

namespace micropython {

PyModule::PyModule(MemoryBlock *led_buffer, std::shared_ptr<LEDBus> bus)
		: led_buffer_(led_buffer), bus_(std::move(bus)) {
}

inline PyModule& PyModule::current() {
	return MicroPython::current().modaurcor_;
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
		{MP_QSTR_values,      MP_ARG_REQUIRED | MP_ARG_OBJ,   {u_obj: MP_OBJ_NULL}},
		{MP_QSTR_length,      MP_ARG_OBJ,                     {u_obj: MP_OBJ_NULL}},

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

		if (value < MIN_FPS || value > MAX_FPS)
			mp_raise_ValueError(MP_ERROR_TEXT("fps out of range"));

		wait_us = 1000000 / value;
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
	uint8_t *buffer = led_buffer_->begin();
	const size_t max_bytes = std::min(bus_->length() * BYTES_PER_LED, led_buffer_->size());
	size_t in_bytes = max_bytes;
	size_t out_bytes = 0;

	mp_buffer_info_t bufinfo;
	bool byte_array = mp_get_buffer(values, &bufinfo, MP_BUFFER_READ) && is_byte_array(bufinfo);

	if (byte_array) {
		in_bytes = std::min(in_bytes, bufinfo.len);

		if (type != OutputType::RGB)
			mp_raise_ValueError(MP_ERROR_TEXT("can only use byte array for RGB values"));
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
		const uint8_t *input = reinterpret_cast<uint8_t *>(bufinfo.buf);
		const size_t buf_bytes = bufinfo.len;
		const size_t rotate_bytes = rotate_length >= 0
			? ((size_t)rotate_length * BYTES_PER_LED)
			: (buf_bytes - (size_t)std::abs(rotate_length) * BYTES_PER_LED);

		if (rotate_bytes > buf_bytes)
			mp_raise_ValueError(MP_ERROR_TEXT("can't rotate by more than the length of byte array"));

		if (buf_bytes % BYTES_PER_LED != 0)
			mp_raise_ValueError(MP_ERROR_TEXT("byte array length must be a multiple of 3 bytes"));

		if (reverse) {
			if (rotate_bytes != 0) {
				size_t available_bytes = std::min(in_bytes, rotate_bytes);

				in_bytes -= available_bytes;
				for (size_t i = rotate_bytes; available_bytes > 0; i -= BYTES_PER_LED) {
					std::memcpy(&buffer[out_bytes], &input[i - BYTES_PER_LED], BYTES_PER_LED);
					out_bytes += BYTES_PER_LED;
					available_bytes -= BYTES_PER_LED;
				}
			}

			for (size_t i = buf_bytes; in_bytes > 0; i -= BYTES_PER_LED) {
				std::memcpy(&buffer[out_bytes], &input[i - BYTES_PER_LED], BYTES_PER_LED);
				out_bytes += BYTES_PER_LED;
				in_bytes -= BYTES_PER_LED;
			}
		} else {
			if (rotate_bytes != 0) {
				size_t available_bytes = std::min(in_bytes, buf_bytes - rotate_bytes);

				std::memcpy(&buffer[out_bytes], &input[rotate_bytes], available_bytes);
				out_bytes += available_bytes;
				in_bytes -= available_bytes;
			}

			if (in_bytes > 0) {
				std::memcpy(&buffer[out_bytes], &input[0], in_bytes);
				out_bytes += in_bytes;
			}
		}
	} else if (rotate_length != 0 || reverse) {
		const size_t values_length = mp_obj_get_int(mp_obj_len(values));
		const size_t abs_rotate_length = rotate_length >= 0
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
					append_led(buffer, type, values_subscr(values, MP_OBJ_NEW_SMALL_INT(i - 1), MP_OBJ_SENTINEL), out_bytes);
					out_bytes += BYTES_PER_LED;
					available_length--;
				}
			}

			for (size_t i = values_length; in_length > 0; i--) {
				append_led(buffer, type, values_subscr(values, MP_OBJ_NEW_SMALL_INT(i - 1), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				in_length--;
			}
		} else {
			size_t available_length = std::min(in_length, values_length - abs_rotate_length);

			in_length -= available_length;
			for (size_t i = abs_rotate_length; available_length > 0; i++) {
				append_led(buffer, type, values_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				available_length--;
			}

			for (size_t i = 0; in_length > 0; i++) {
				append_led(buffer, type, values_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL), out_bytes);
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

			std::memcpy(&buffer[out_bytes], &buffer[0], available_bytes);
			out_bytes += available_bytes;
		}
	}

	bus_->profile(profile).transform(buffer, out_bytes);

	if (wait_us) {
		uint64_t start_us = bus_->last_update_us() + wait_us - TIMING_DELAY_US;
		uint64_t now_us = current_time_us();

		if (start_us > now_us)
			mp_hal_delay_us(start_us - now_us);
	}

	bus_->write(buffer, out_bytes);

	return MP_ROM_NONE;
}

void PyModule::append_led(uint8_t *buffer, OutputType type, mp_obj_t item, size_t offset) {
	// TODO parse item
}

void PyModule::hsv_to_rgb(mp_int_t hue, mp_int_t saturation, mp_int_t value,
		std::array<uint8_t,3> &rgb) {
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

inline void PyModule::exp_hsv_to_rgb(mp_int_t expanded_hue, mp_int_t saturation,
		mp_int_t value, std::array<uint8_t,3> &rgb) {
	mp_int_t hue;

	if (expanded_hue < EXPANDED_HUE_LEFT_RANGE) {
		hue = expanded_hue / EXPANDED_HUE_TIMES;
	} else {
		hue = expanded_hue - EXPANDED_HUE_RIGHT_OFFSET;
	}

	hsv_to_rgb(hue, saturation, value, rgb);
}

void PyModule::hsv_to_rgb(size_t n_args, const mp_obj_t *args, bool exp,
		std::array<uint8_t,3> &rgb) {
	mp_int_t hue = mp_obj_get_int(args[0]);
	mp_int_t saturation = MAX_SATURATION;
	mp_int_t value = MAX_VALUE;

	switch (n_args) {
	case 3:
		saturation = int_constrain(mp_obj_get_int(args[1]), MAX_SATURATION);
		[[fallthrough]];
	case 2:
		value = int_constrain(mp_obj_get_int(args[n_args - 1]), MAX_VALUE);
		break;
	}

	if (exp) {
		PyModule::exp_hsv_to_rgb(hue % EXPANDED_HUE_RANGE, saturation, value, rgb);
	} else {
		PyModule::hsv_to_rgb(hue % HUE_RANGE, saturation, value, rgb);
	}
}

void PyModule::hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args, bool exp) {
	enum { ARG_buffer, ARG_offset };
	mp_buffer_info_t bufinfo;
	size_t offset;

	mp_get_buffer_raise(args[ARG_buffer], &bufinfo, MP_BUFFER_WRITE);

	if (!is_byte_array(bufinfo))
		mp_raise_TypeError(MP_ERROR_TEXT("buffer must be a byte array"));

	if (!mp_obj_is_int(args[ARG_offset])) {
			mp_raise_TypeError(MP_ERROR_TEXT("offset must be an int"));
	} else {
		mp_int_t value = mp_obj_get_int(args[ARG_offset]);

		offset = (size_t)value;

		if (value < 0 || offset > bufinfo.len)
			mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));
	}

	if (bufinfo.len - offset < BYTES_PER_LED)
		mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));

	hsv_to_rgb(n_args - 2, &args[2], exp, *reinterpret_cast<std::array<uint8_t,3>*>(
		&reinterpret_cast<uint8_t*>(bufinfo.buf)[offset]));
}

mp_obj_t PyModule::hsv_to_rgb_int(size_t n_args, const mp_obj_t *args, bool exp) {
	std::array<uint8_t,3> rgb_bytes;

	hsv_to_rgb(n_args, args, exp, rgb_bytes);

	static_assert(MP_SMALL_INT_FITS(0xFFFFFF), "small int overflow");
	return MP_OBJ_NEW_SMALL_INT((rgb_bytes[0] << 16) | (rgb_bytes[1] << 8) | rgb_bytes[2]);
}

mp_obj_t PyModule::hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args, bool exp) {
	std::array<uint8_t,3> rgb_bytes;
	mp_obj_tuple_t *tuple = reinterpret_cast<mp_obj_tuple_t *>(mp_obj_new_tuple(rgb_bytes.size(), nullptr));

	hsv_to_rgb(n_args, args, exp, rgb_bytes);

	for (size_t i = 0; i < rgb_bytes.size(); i++)
		tuple->items[i] = MP_OBJ_NEW_SMALL_INT(rgb_bytes[i]);

	return tuple;
}

void PyModule::rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, std::array<mp_int_t,3> &hsv) {
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

inline void PyModule::rgb_to_exp_hsv(uint8_t r, uint8_t g, uint8_t b, std::array<mp_int_t,3> &hsv) {
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
		PyModule::rgb_to_exp_hsv(r, g, b, hsv);
	} else {
		PyModule::rgb_to_hsv(r, g, b, hsv);
	}

	for (size_t i = 0; i < hsv.size(); i++)
		tuple->items[i] = MP_OBJ_NEW_SMALL_INT(hsv[i]);

	return tuple;
}

} // namespace micropython

} // namespace aurcor
