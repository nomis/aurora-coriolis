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
# include <py/binary.h>
# include <py/obj.h>
#endif

typedef struct {
	mp_obj_base_t base;
	size_t len;
	uintptr_t items[4];
} aurcor_version_t;

extern const aurcor_version_t aurcor_version_obj;

mp_obj_t aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_rgb_obj);

mp_obj_t aurcor_output_hsb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_hsb_obj);

mp_obj_t aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
MP_DECLARE_CONST_FUN_OBJ_KW(aurcor_output_defaults_obj);

#ifdef __cplusplus
} // extern "C"

#ifndef NO_QSTR
# include <uuid/log.h>

# include "led_bus.h"
# include "led_profiles.h"
# include "memory_pool.h"
#endif

namespace aurcor {

namespace micropython {

class PyModule {
public:
	enum class OutputType {
		RGB,
		HSB,
		DEFAULTS,
	};

	PyModule(MemoryBlock *led_buffer);

	mp_obj_t output_leds(OutputType type, size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);

private:
	static constexpr size_t BYTES_PER_LED = LEDBus::BYTES_PER_LED;
	static constexpr enum led_profile_id DEFAULT_PROFILE = LED_PROFILE_NORMAL;
	static constexpr mp_int_t MIN_FPS = 1;
	static constexpr mp_int_t MAX_FPS = 1000;
	static constexpr mp_int_t MIN_WAIT_MS = 1;
	static constexpr mp_int_t MAX_WAIT_MS = 1000;
	static constexpr uint32_t DEFAULT_WAIT_US = 0;
	static constexpr bool DEFAULT_REPEAT = false;
	static constexpr bool DEFAULT_REVERSE = false;

	friend mp_obj_t ::aurcor_output_rgb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_hsb(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	friend mp_obj_t ::aurcor_output_defaults(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs);
	static PyModule& current();

	static inline bool is_byte_array(const mp_buffer_info_t &bufinfo) {
		switch (bufinfo.typecode) {
		case BYTEARRAY_TYPECODE:
		case 'b':
		case 'B':
			return true;
		}

		return false;
	}

	static void append_bytes(uint8_t *buffer, OutputType type,
		const mp_buffer_info_t &bufinfo, size_t offset, size_t bytes,
		size_t out_bytes);
	static void append_led(uint8_t *buffer, OutputType type, mp_obj_t item, size_t offset);
	static void hsb_to_rgb(uint8_t *src, size_t src_offset, uint8_t *dst, size_t dst_offset);

	MemoryBlock *led_buffer_;
	enum led_profile_id profile_{DEFAULT_PROFILE};
	uint32_t wait_us_{0};
	bool repeat_{DEFAULT_REPEAT};
	bool reverse_{DEFAULT_REVERSE};
};

} // namespace micropython

} // namespace aurcor
#endif
