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

#include <Arduino.h>
#ifndef ENV_NATIVE
# include <esp_idf_version.h>
# include <esp_task_wdt.h>
#endif

#include "aurcor/app.h"

static aurcor::App application;

void setup() {
#ifndef ENV_NATIVE
	heap_caps_malloc_extmem_enable(0);

	/* Disable the idle task watchdog because it won't have time to run
	 * if a MicroPython script is busy all the time.
	 */
# if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
	esp_task_wdt_config_t wdt_config = {
		.timeout_ms = CONFIG_ESP_TASK_WDT_TIMEOUT_S * 1000,
		.idle_core_mask = 0,
		.trigger_panic = CONFIG_ESP_TASK_WDT_PANIC ? true : false,
	};
	esp_task_wdt_reconfigure(&wdt_config);
# else
#  ifdef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0
	esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0));
#  endif
#  ifdef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1
	esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(1));
#  endif
# endif
#endif

	try {
		application.start();
	} catch (...) {
#ifdef ENV_NATIVE
		throw;
#endif
		application.exception(F("setup"));
	}
}

void loop() {
	try {
		application.loop();
		::yield();
	} catch (...) {
#ifdef ENV_NATIVE
		throw;
#endif
		application.exception(F("loop"));
	}
}
