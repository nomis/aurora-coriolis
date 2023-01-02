/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
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

#include <sys/time.h>

#include "py/obj.h"
#include "py/objstr.h"
#include "py/mpstate.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "shared/timeutils/timeutils.h"
#include "shared/runtime/pyexec.h"
#include "mphalport.h"

#include <esp_timer.h>

mp_uint_t mp_hal_ticks_ms(void) {
    return esp_timer_get_time() / 1000ULL;
}

mp_uint_t mp_hal_ticks_us(void) {
    return esp_timer_get_time();
}

#ifdef ENV_NATIVE
# include <time.h>
# include <unistd.h>

void mp_hal_delay_ms(mp_uint_t ms) {
    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = ms * 1000000,
    };

    nanosleep(&ts, NULL);
}

void mp_hal_delay_us(mp_uint_t us) {
    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = us * 1000,
    };

    nanosleep(&ts, NULL);
}
#else
# include "freertos/FreeRTOS.h"
# include "freertos/task.h"

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t us = ms * 1000;
    uint64_t dt;
    uint64_t t0 = esp_timer_get_time();
    for (;;) {
        mp_handle_pending(true);
        MP_THREAD_GIL_EXIT();
        uint64_t t1 = esp_timer_get_time();
        dt = t1 - t0;
        if (dt + portTICK_PERIOD_MS * 1000 >= us) {
            // doing a vTaskDelay would take us beyond requested delay time
            taskYIELD();
            MP_THREAD_GIL_ENTER();
            t1 = esp_timer_get_time();
            dt = t1 - t0;
            break;
        } else {
            ulTaskNotifyTake(pdFALSE, 1);
            MP_THREAD_GIL_ENTER();
        }
    }
    if (dt < us) {
        // do the remaining delay accurately
        mp_hal_delay_us(us - dt);
    }
}

void mp_hal_delay_us(mp_uint_t us) {
    // these constants are tested for a 240MHz clock
    const uint32_t this_overhead = 5;
    const uint32_t pend_overhead = 150;

    // return if requested delay is less than calling overhead
    if (us < this_overhead) {
        return;
    }
    us -= this_overhead;

    uint64_t t0 = esp_timer_get_time();
    for (;;) {
        uint64_t dt = esp_timer_get_time() - t0;
        if (dt >= us) {
            return;
        }
        if (dt + pend_overhead < us) {
            // we have enough time to service pending events
            // (don't use MICROPY_EVENT_POLL_HOOK because it also yields)
            mp_handle_pending(true);
        }
    }
}
#endif

int32_t mp_hal_time_s(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int32_t seconds = tv.tv_sec;
    #if !MICROPY_EPOCH_IS_1970
    seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
    #endif
    return seconds;
}

int64_t mp_hal_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int32_t seconds = tv.tv_sec;
    #if !MICROPY_EPOCH_IS_1970
    seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
    #endif
    int64_t ms = seconds * 1000LL;
    ms += tv.tv_usec / 1000LL;
    return ms;
}

int64_t mp_hal_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int32_t seconds = tv.tv_sec;
    #if !MICROPY_EPOCH_IS_1970
    seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
    #endif
    int64_t us = seconds * 1000000LL;
    us += tv.tv_usec;
    return us;
}

int64_t mp_hal_time_ns(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int32_t seconds = tv.tv_sec;
    #if !MICROPY_EPOCH_IS_1970
    seconds = (uint32_t)seconds - TIMEUTILS_SECONDS_1970_TO_2000;
    #endif
    int64_t ns = seconds * 1000000000LL;
    ns += tv.tv_usec * 1000LL;
    return ns;
}
