/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2021 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>

// Options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL                (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// Compiler configuration
#define MICROPY_ENABLE_COMPILER                 (1)

// Python internal features
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_DETAILED)

// Type definitions for the specific machine

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// Need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
	const char *readline_hist[8];

#define MICROPY_PLATFORM_VERSION "IDF" IDF_VER

#define MICROPY_HW_BOARD_NAME "ESP32 module"
#define MICROPY_HW_MCU_NAME "ESP32"

#define MICROPY_INSTANCE_PER_THREAD 1
#define MICROPY_ENABLE_GC 1
#define MICROPY_ENABLE_PYSTACK 1

#define MICROPY_HELPER_REPL 1
#define MICROPY_KBD_EXCEPTION 1
#define MICROPY_HAL_HAS_VT100 1

extern mp_uint_t mp_hal_begin_atomic_section(void);
extern void mp_hal_end_atomic_section(void);

#define MICROPY_BEGIN_ATOMIC_SECTION() mp_hal_begin_atomic_section()
#define MICROPY_END_ATOMIC_SECTION(state) mp_hal_end_atomic_section()
