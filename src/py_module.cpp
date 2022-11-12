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

using aurcor::MicroPython;
using aurcor::micropython::PyModule;

extern "C" {

mp_obj_t aurcor_hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args) {
	enum { ARG_buffer, ARG_offset };
	mp_buffer_info_t bufinfo;
	size_t offset;

	mp_get_buffer_raise(args[ARG_buffer], &bufinfo, MP_BUFFER_WRITE);

	if (!PyModule::is_byte_array(bufinfo))
		mp_raise_TypeError(MP_ERROR_TEXT("buffer must be a byte array"));

	if (!mp_obj_is_int(args[ARG_offset])) {
			mp_raise_TypeError(MP_ERROR_TEXT("offset must be an int"));
	} else {
		mp_int_t value = mp_obj_get_int(args[ARG_offset]);

		offset = (size_t)value;

		if (value < 0 || offset > bufinfo.len)
			mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));
	}

	if (bufinfo.len - offset < PyModule::BYTES_PER_LED)
		mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("buffer index out of range"));

	PyModule::hsv_to_rgb(n_args - 2, &args[2], &reinterpret_cast<uint8_t *>(bufinfo.buf)[offset]);

	return MP_ROM_NONE;
}

mp_obj_t aurcor_hsv_to_rgb_int(size_t n_args, const mp_obj_t *args) {
	std::array<uint8_t,PyModule::BYTES_PER_LED> rgb_bytes;

	PyModule::hsv_to_rgb(n_args, args, rgb_bytes.data());

	static_assert(MP_SMALL_INT_FITS(0xFFFFFF), "small int overflow");
	return MP_OBJ_NEW_SMALL_INT((rgb_bytes[0] << 16) | (rgb_bytes[1] << 8) | rgb_bytes[2]);
}

mp_obj_t aurcor_hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args) {
	std::array<uint8_t,PyModule::BYTES_PER_LED> rgb_bytes;
	std::array<mp_obj_t,PyModule::BYTES_PER_LED> rgb_tuple;

	PyModule::hsv_to_rgb(n_args, args, rgb_bytes.data());

	rgb_tuple[0] = MP_OBJ_NEW_SMALL_INT(rgb_bytes[0]);
	rgb_tuple[1] = MP_OBJ_NEW_SMALL_INT(rgb_bytes[1]);
	rgb_tuple[2] = MP_OBJ_NEW_SMALL_INT(rgb_bytes[2]);

	return mp_obj_new_tuple(rgb_tuple.size(), rgb_tuple.data());
}

mp_obj_t aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::RGB, n_args, args, kwargs);
}

mp_obj_t aurcor_output_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	return PyModule::current().output_leds(PyModule::OutputType::HSV, n_args, args, kwargs);
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
		const size_t rotate_bytes = rotate_length > 0
			? ((size_t)rotate_length * BYTES_PER_LED)
			: (buf_bytes - (size_t)std::abs(rotate_length) * BYTES_PER_LED);

		if (rotate_bytes > buf_bytes)
			mp_raise_ValueError(MP_ERROR_TEXT("can't rotate by more than the length of byte array"));

		if (!reverse) {
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
		} else {
			if (rotate_bytes != 0) {
				size_t available_bytes = std::min(in_bytes, rotate_bytes);

				in_bytes -= available_bytes;
				for (size_t i = rotate_bytes; available_bytes > 0; i -= BYTES_PER_LED) {
					std::memcpy(&buffer[out_bytes], &input[i], BYTES_PER_LED);
					out_bytes += BYTES_PER_LED;
					available_bytes -= BYTES_PER_LED;
				}
			}

			for (size_t i = buf_bytes; in_bytes > 0; i -= BYTES_PER_LED) {
				std::memcpy(&buffer[out_bytes], &input[i], BYTES_PER_LED);
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

			for (size_t i = abs_rotate_length; available_length > 0; i++) {
				append_led(buffer, type, values_subscr(values, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL), out_bytes);
				out_bytes += BYTES_PER_LED;
				available_length--;
			}
			in_length -= available_length;

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

	bus_->write(buffer, out_bytes);

	return MP_ROM_NONE;
}

void PyModule::append_led(uint8_t *buffer, OutputType type, mp_obj_t item, size_t offset) {
	// TODO parse item
}

void PyModule::hsv_to_rgb(mp_int_t hue360, mp_float_t saturation, mp_float_t value, uint8_t *rgb) {
	mp_float_t hi;
	mp_float_t hf = std::modf(hue360 / 60.0, &hi);
	mp_float_t s = float_0to1(saturation);
	mp_float_t v = float_0to1(value);
	mp_float_t p = v * (1 - s);
	mp_float_t q = v * (1 - s * hf);
	mp_float_t t = v * (1 - s * (1 - hf));
	mp_float_t r = 0;
	mp_float_t g = 0;
	mp_float_t b = 0;

	switch (std::lround(hi) % 6) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	case 5:
		r = v;
		g = p;
		b = q;
		break;
	}

	rgb[0] = int_to_u8(r * 255);
	rgb[1] = int_to_u8(g * 255);
	rgb[2] = int_to_u8(b * 255);
}

void PyModule::exp_hsv_to_rgb(mp_float_t expanded_hue1, mp_float_t saturation, mp_float_t value, uint8_t *rgb) {
	mp_float_t integral;
	mp_int_t hue360;

	expanded_hue1 = std::modf(expanded_hue1, &integral);

	if (expanded_hue1 < EXPANDED_HUE_F_RANGE1) {
		hue360 = std::lround(expanded_hue1 * EXPANDED_HUE_MULTIPLIER1);
	} else {
		hue360 = EXPANDED_HUE_I_RANGE1 + std::lround((expanded_hue1 - EXPANDED_HUE_F_RANGE1) * EXPANDED_HUE_MULTIPLIER2);
	}

	hsv_to_rgb(hue360, saturation, value, rgb);
}

void PyModule::hsv_to_rgb(size_t n_args, const mp_obj_t *args, uint8_t *rgb) {
	mp_float_t saturation = 1.0;
	mp_float_t value = 1.0;

	if (n_args >= 2) {
		if (mp_obj_is_int(args[n_args - 1])) {
			value = mp_obj_get_int(args[n_args - 1]) / MAX_VALUE;
		} else if (mp_obj_is_float(args[n_args - 1])) {
			value = mp_obj_get_float(args[n_args - 1]);
		} else {
			mp_raise_TypeError(MP_ERROR_TEXT("value must be an int or a float"));
		}
	}

	if (n_args == 3) {
		if (mp_obj_is_int(args[1])) {
			saturation = mp_obj_get_int(args[1]) / MAX_SATURATION;
		} else if (mp_obj_is_float(args[1])) {
			saturation = mp_obj_get_float(args[1]);
		} else {
			mp_raise_TypeError(MP_ERROR_TEXT("saturation must be an int or a float"));
		}
	}

	if (mp_obj_is_int(args[0])) {
		PyModule::hsv_to_rgb(mp_obj_get_int(args[0]), saturation, value, rgb);
	} else if (mp_obj_is_float(args[0])) {
		PyModule::exp_hsv_to_rgb(mp_obj_get_float(args[0]), saturation, value, rgb);
	} else {
		mp_raise_TypeError(MP_ERROR_TEXT("hue must be an int or a float"));
	}
}

} // namespace micropython

} // namespace aurcor
