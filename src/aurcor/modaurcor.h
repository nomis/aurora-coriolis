/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2024  Simon Arlott
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
# include <py/binary.h>
# include <py/obj.h>
#endif

typedef struct {
	mp_obj_base_t base;
	size_t len;
	uintptr_t items[4];
} aurcor_version_t;

extern const aurcor_version_t aurcor_version_obj;

mp_obj_t aurcor_hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_buffer_obj);

mp_obj_t aurcor_hsv_to_rgb_int(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_int_obj);

mp_obj_t aurcor_hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_hsv_to_rgb_tuple_obj);


mp_obj_t aurcor_exp_hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_exp_hsv_to_rgb_buffer_obj);

mp_obj_t aurcor_exp_hsv_to_rgb_int(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_exp_hsv_to_rgb_int_obj);

mp_obj_t aurcor_exp_hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_exp_hsv_to_rgb_tuple_obj);


mp_obj_t aurcor_rgb_to_hsv_tuple(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_rgb_to_hsv_tuple_obj);

mp_obj_t aurcor_rgb_to_exp_hsv_tuple(size_t n_args, const mp_obj_t *args);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(aurcor_rgb_to_exp_hsv_tuple_obj);


mp_obj_t aurcor_length(void);
MP_DECLARE_CONST_FUN_OBJ_0(aurcor_length_obj);

mp_obj_t aurcor_default_fps(void);
MP_DECLARE_CONST_FUN_OBJ_0(aurcor_default_fps_obj);

mp_obj_t aurcor_register_config(mp_obj_t dict);
MP_DECLARE_CONST_FUN_OBJ_1(aurcor_register_config_obj);

mp_obj_t aurcor_config(mp_obj_t dict);
MP_DECLARE_CONST_FUN_OBJ_1(aurcor_config_obj);


mp_obj_t aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_rgb_obj);

mp_obj_t aurcor_output_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_hsv_obj);

mp_obj_t aurcor_output_exp_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_exp_hsv_obj);

mp_obj_t aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_defaults_obj);


mp_obj_t aurcor_next_ticks30_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_ticks30_ms_obj);

mp_obj_t aurcor_next_ticks64_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_ticks64_ms_obj);

mp_obj_t aurcor_next_ticks64_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_ticks64_us_obj);

mp_obj_t aurcor_next_time(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_time_obj);

mp_obj_t aurcor_next_time_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_time_ms_obj);

mp_obj_t aurcor_next_time_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_next_time_us_obj);


mp_obj_t aurcor_time(void);
MP_DECLARE_CONST_FUN_OBJ_0(aurcor_time_obj);

mp_obj_t aurcor_time_ms(void);
MP_DECLARE_CONST_FUN_OBJ_0(aurcor_time_ms_obj);

mp_obj_t aurcor_time_us(void);
MP_DECLARE_CONST_FUN_OBJ_0(aurcor_time_us_obj);


mp_obj_t aurcor_udp_receive(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_udp_receive_obj);

#ifdef __cplusplus
} // extern "C"

#ifndef NO_QSTR
# include <uuid/log.h>

# include "led_bus.h"
# include "led_bus_format.h"
# include "led_profiles.h"

# include <memory>
# include <limits>
#endif

namespace aurcor {

class MemoryBlock;
class Preset;

namespace micropython {

class PyModule {
public:
	static constexpr size_t BYTES_PER_LED = LEDBus::BYTES_PER_LED;
	static constexpr mp_int_t HUE_RANGE = AURCOR_HUE_RANGE;
	static constexpr mp_int_t MAX_SATURATION = AURCOR_MAX_SATURATION;
	static constexpr mp_int_t MAX_VALUE = AURCOR_MAX_VALUE;
	static constexpr mp_int_t EXPANDED_HUE_RANGE = AURCOR_EXP_HUE_RANGE;
	static constexpr ssize_t MIN_SLENGTH = (std::numeric_limits<ssize_t>::min() + (ssize_t)BYTES_PER_LED - 1) / (ssize_t)BYTES_PER_LED;
	static constexpr ssize_t MAX_SLENGTH = std::numeric_limits<ssize_t>::max() / BYTES_PER_LED;
	static constexpr size_t MAX_ULENGTH = std::numeric_limits<size_t>::max() / BYTES_PER_LED;
	static_assert(-(MIN_SLENGTH * (ssize_t)BYTES_PER_LED) > 0, "Minimum signed length can't be converted to a positive value");
	static_assert((MIN_SLENGTH * (ssize_t)BYTES_PER_LED) > std::numeric_limits<ssize_t>::min(), "Minimum signed length can't be converted to a positive value");
	static_assert(MAX_LEDS <= MAX_ULENGTH, "Too many LEDs");

	enum class OutputType {
		RGB,
		HSV,
		EXP_HSV,
	};

	static void hsv_to_rgb(mp_int_t hue, mp_int_t saturation, mp_int_t value, uint8_t rgb[3]);
	static void exp_hsv_to_rgb(mp_int_t expanded_hue, mp_int_t saturation, mp_int_t value, uint8_t rgb[3]);

	static void hsv_to_rgb(size_t n_args, const mp_obj_t *args, bool exp, uint8_t rgb[3]);
	static void hsv_to_rgb_buffer(size_t n_args, const mp_obj_t *args, bool exp);
	static mp_obj_t hsv_to_rgb_int(size_t n_args, const mp_obj_t *args, bool exp);
	static mp_obj_t hsv_to_rgb_tuple(size_t n_args, const mp_obj_t *args, bool exp);

	static void rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b, mp_int_t hsv[3]);
	static void rgb_to_exp_hsv(uint8_t r, uint8_t g, uint8_t b, mp_int_t hsv[3]);

	static mp_obj_t rgb_to_hsv_tuple(size_t n_args, const mp_obj_t *args, bool exp);

	PyModule(MemoryBlock *led_buffer, std::shared_ptr<LEDBus> bus, Preset &preset);

	mp_obj_t length();
	mp_obj_t default_fps();
	mp_obj_t register_config(mp_obj_t dict);
	mp_obj_t config(mp_obj_t dict);
	mp_obj_t output_leds(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs, OutputType type, bool set_defaults);

	mp_obj_t next_ticks30_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	mp_obj_t next_ticks64_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	mp_obj_t next_ticks64_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	mp_obj_t next_time(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	mp_obj_t next_time_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	mp_obj_t next_time_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);

	mp_obj_t udp_receive(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);

private:
	static constexpr size_t TIMING_DELAY_US = 10;
	static constexpr enum led_profile_id DEFAULT_PROFILE = LED_PROFILE_NORMAL;
	static constexpr mp_int_t MAX_WAIT_MS = 1000;
	static constexpr mp_int_t MAX_WAIT_US = MAX_WAIT_MS * 1000;
public:
	static constexpr mp_int_t MIN_FPS = 1000 / MAX_WAIT_MS;
	static constexpr mp_int_t MAX_FPS = 1000;
private:
	static constexpr long DEFAULT_WAIT_US = -1;
	static constexpr bool DEFAULT_REPEAT = false;
	static constexpr bool DEFAULT_REVERSE = false;

	static constexpr mp_int_t EXPANDED_HUE_TIMES = AURCOR_EXP_HUE_TIMES;
	static constexpr mp_int_t EXPANDED_HUE_SIZE = AURCOR_EXP_HUE_SIZE;
	static constexpr mp_int_t EXPANDED_HUE_LEFT_RANGE = EXPANDED_HUE_SIZE * EXPANDED_HUE_TIMES;
	static constexpr mp_int_t EXPANDED_HUE_RIGHT_OFFSET = EXPANDED_HUE_SIZE * (EXPANDED_HUE_TIMES - 1);
	static_assert(EXPANDED_HUE_TIMES > 1, "Invalid expanded hue times");
	static_assert(EXPANDED_HUE_SIZE == (EXPANDED_HUE_SIZE % HUE_RANGE), "Invalid expanded hue size");

	static constexpr bool HSV_TO_RGB_USE_FLOAT = false;
	static constexpr bool RGB_TO_HSV_USE_FLOAT = false;

	friend mp_obj_t ::aurcor_length();
	friend mp_obj_t ::aurcor_default_fps();
	friend mp_obj_t ::aurcor_register_config(mp_obj_t dict);
	friend mp_obj_t ::aurcor_config(mp_obj_t dict);
	friend mp_obj_t ::aurcor_next_ticks30_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_next_ticks64_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_next_ticks64_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_next_time(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_next_time_ms(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_next_time_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_exp_hsv(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_udp_receive(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	static PyModule& current();

	static void append_led(OutputType type, uint8_t *buffer, size_t offset, mp_obj_t item);
	static mp_int_t hue_obj_to_int(mp_obj_t hue, bool exp);
	static mp_int_t saturation_obj_to_int(mp_obj_t saturation);
	static mp_int_t value_obj_to_int(mp_obj_t value);

	long calc_wait_us(mp_obj_t fps_obj, mp_obj_t wait_ms_obj, mp_obj_t wait_us_obj, bool set_defaults);
	void next_wait_us(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs,
		uint64_t &now_us, uint64_t &start_us);
	void next_timeofday(struct timeval &tv, uint64_t offset_us);

	MemoryBlock *led_buffer_;
	std::shared_ptr<LEDBus> bus_;
	size_t bus_length_;
	LEDBusFormat bus_format_;
	unsigned int bus_default_fps_;
	Preset &preset_;

	enum led_profile_id profile_{DEFAULT_PROFILE};
	long wait_us_{DEFAULT_WAIT_US};
	bool repeat_{DEFAULT_REPEAT};
	bool reverse_{DEFAULT_REVERSE};

	bool bus_written_{false};
	bool config_used_{false};
};

} // namespace micropython

} // namespace aurcor
#endif
