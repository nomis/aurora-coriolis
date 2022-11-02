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
#define MICROPY_CONFIG_ROM_LEVEL MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES

// Compiler configuration
#define MICROPY_ENABLE_COMPILER 1

// Python internal features
#define MICROPY_OPT_COMPUTED_GOTO 1
#define MICROPY_PY_ALL_INPLACE_SPECIAL_METHODS 1
#define MICROPY_PY_SYS_EXC_INFO 1
#define MICROPY_PY_UTIME_MP_HAL 1

#define MICROPY_ENABLE_EXTERNAL_IMPORT 0
#define MICROPY_STREAMS_NON_BLOCK 0
#define MICROPY_STREAMS_POSIX_API 0
#define MICROPY_ENABLE_SCHEDULER 0
#define MICROPY_PY_MICROPYTHON_MEM_INFO 0
#define MICROPY_PY_IO 0
#define MICROPY_PY_STRUCT 0
#define MICROPY_PY_SYS 0
#define MICROPY_PY_SYS_PATH_ARGV_DEFAULTS 0
#define MICROPY_PY_SYS_MAXSIZE 0
#define MICROPY_PY_SYS_MODULES 0
#define MICROPY_PY_SYS_EXIT 0
#define MICROPY_PY_SYS_PS1_PS2 0
#define MICROPY_PY_SYS_STDFILES 0
#define MICROPY_PY_SYS_STDIO_BUFFER 0
#define MICROPY_PY_SYS_TRACEBACKLIMIT 0
#define MICROPY_PY_SYS_ATTR_DELEGATION 0
#define MICROPY_PY_UERRNO 0
#define MICROPY_PY_USELECT 0
#define MICROPY_PY_UASYNCIO 0
#define MICROPY_PY_UCTYPES 0
#define MICROPY_PY_UZLIB 0
#define MICROPY_PY_UJSON 0
#define MICROPY_PY_UOS 0
#define MICROPY_PY_URE 0
#define MICROPY_PY_UHASHLIB 0
#define MICROPY_PY_UCRYPTOLIB 0
#define MICROPY_PY_UBINASCII 0
#define MICROPY_PY_MACHINE 0
#define MICROPY_PY_USSL 0
#define MICROPY_PY_UWEBSOCKET 0
#define MICROPY_PY_FRAMEBUF 0
#define MICROPY_PY_BTREE 0
#define MICROPY_PY_ONEWIRE 0

// Type definitions for the specific machine
typedef intptr_t mp_int_t;
typedef uintptr_t mp_uint_t;
typedef long mp_off_t;

#define MICROPY_OBJ_REPR MICROPY_OBJ_REPR_C
#define MICROPY_FLOAT_IMPL MICROPY_FLOAT_IMPL_FLOAT

// Need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
	const char *readline_hist[8];

#define MICROPY_PLATFORM_VERSION "IDF" IDF_VER
#define MICROPY_PY_SYS_PLATFORM "esp32"
#define MICROPY_HW_BOARD_NAME "ESP32 module"
#define MICROPY_HW_MCU_NAME "ESP32"

#define MICROPY_ALLOC_PATH_MAX 64
#define MICROPY_ENABLE_GC 1
#define MICROPY_ENABLE_PYSTACK 1
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF 1
#define MICROPY_EPOCH_IS_1970 0
#define MICROPY_ERROR_REPORTING MICROPY_ERROR_REPORTING_DETAILED
#define MICROPY_HAL_HAS_VT100 1
#define MICROPY_HELPER_REPL 1
#define MICROPY_INSTANCE_PER_THREAD 1
#define MICROPY_KBD_EXCEPTION 1
#define MICROPY_LONGINT_IMPL MICROPY_LONGINT_IMPL_MPZ
#ifndef ENV_NATIVE
# define MICROPY_PY_URANDOM_SEED_INIT_FUNC esp_random()
#endif
#define MICROPY_REPL_SUPPORT_RAW_PASTE 0
#define MICROPY_USE_INTERNAL_PRINTF 0
#define MICROPY_WARNINGS 1

extern mp_uint_t mp_hal_begin_atomic_section(void);
extern void mp_hal_end_atomic_section(void);

#define MICROPY_BEGIN_ATOMIC_SECTION() mp_hal_begin_atomic_section()
#define MICROPY_END_ATOMIC_SECTION(state) mp_hal_end_atomic_section()
