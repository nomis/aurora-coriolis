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

#include "aurcor/led_bus.h"

#include <Arduino.h>

#include <esp_timer.h>
#include <freertos/semphr.h>

#include <algorithm>
#include <cstring>

#include <uuid/log.h>

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-bus";

namespace aurcor {

uuid::log::Logger LEDBus::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::LPR};

LEDBus::LEDBus(const __FlashStringHelper *name) : name_(name), profiles_(name) {
	semaphore_ = xSemaphoreCreateBinary();
	if (semaphore_) {
		if (xSemaphoreGive(semaphore_) != pdTRUE) {
			logger_.emerg(F("[%S] Semaphore init failed"), name);
			vSemaphoreDelete(semaphore_);
			semaphore_ = nullptr;
		}
	}
}

LEDBus::~LEDBus() {
	if (semaphore_) {
		vSemaphoreDelete(semaphore_);
	}
}

uint64_t LEDBus::last_update_us() const {
	return last_update_us_;
}

bool LEDBus::ready() const {
	return !busy_;
}

void LEDBus::write(const uint8_t *data, size_t size) {
	if (!semaphore_)
		return;

	if (xSemaphoreTake(semaphore_, SEMAPHORE_TIMEOUT_TICKS) != pdTRUE) {
		logger_.emerg(F("[%S] Semaphore take timeout"), name_);
		return;
	}

	busy_ = true;
	last_update_us_ = esp_timer_get_time();

	start(data, size);
}

void LEDBus::finish() {
	busy_ = false;
	if (xSemaphoreGive(semaphore_) != pdTRUE)
		logger_.emerg(F("[%S] Semaphore give failed"), name_);
}

IRAM_ATTR void LEDBus::finish_isr() {
	BaseType_t xHigherPriorityTaskWoken{pdFALSE};

	busy_ = false;
	xSemaphoreGiveFromISR(semaphore_, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

NullLEDBus::NullLEDBus(const __FlashStringHelper *name) : LEDBus(name) {
	length(MAX_LEDS);
}

void NullLEDBus::start(const uint8_t *data, size_t length) {
	finish();
}

ByteBufferLEDBus::ByteBufferLEDBus(const __FlashStringHelper *name) : LEDBus(name) {
}

void ByteBufferLEDBus::start(const uint8_t *data, size_t size) {
	const size_t max_bytes = length() * BYTES_PER_LED;
	const bool reverse_order = reverse();

	size = std::min(max_bytes, size);

	if (reverse_order) {
		for (size_t out_bytes = size - BYTES_PER_LED, in_bytes = 0; in_bytes < size;
				out_bytes -= BYTES_PER_LED, in_bytes += BYTES_PER_LED)
			std::memcpy(&buffer_[out_bytes], &data[in_bytes], BYTES_PER_LED);
	} else {
		std::memcpy(&buffer_[0], data, size);
	}

	if (size < max_bytes) {
		/*
		 * If the length has increased but the script isn't aware of this yet,
		 * we need to turn off the extra LEDs or they'll have stale values.
		 *
		 * If the LED profile has changed then we need all of the original
		 * values to be able to transform them but the LEDBus doesn't have that
		 * information. The original values need to be buffered somewhere and
		 * that is delegated to the script by not allowing partial writes.
		 */
		if (reverse_order) {
			std::memset(&buffer_[0], 0, max_bytes - size);
		} else {
			std::memset(&buffer_[size], 0, max_bytes - size);
		}
	}

	pos_ = &buffer_[0];
	/*
	 * To ensure consistency in the update rate regardless of where the changes
	 * are, we always write everything. (It would be possible to determine where
	 * the last change is in the buffer before overwriting it.)
	 */
	bytes_ = max_bytes;
	transmit();
}

} // namespace aurcor
