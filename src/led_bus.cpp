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

#include <algorithm>
#include <thread>
#include <vector>

#include <uuid/common.h>
#include <uuid/log.h>

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "led-bus";

namespace aurcor {

uuid::log::Logger LEDBus::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::LPR};

LEDBus::LEDBus(const __FlashStringHelper *name) : name_(name) {
}

uint64_t LEDBus::last_update_ms() const {
	std::lock_guard<std::mutex> lock{mutex_};
	return last_update_ms_;
}

void LEDBus::write(std::vector<uint8_t> data) {
	std::unique_lock<std::mutex> lock{mutex_};

	while (busy_)
		ready_.wait(lock);

	last_update_ms_ = uuid::get_uptime_ms();
	busy_ = true;

	start(lock, data.data(), (data.size() / BYTES_PER_LED) * BYTES_PER_LED);
}

void LEDBus::finish() {
	std::lock_guard<std::mutex> lock{mutex_};

	busy_ = false;
	ready_.notify_all();
}

NullLEDBus::NullLEDBus(const __FlashStringHelper *name) : LEDBus(name) {
	length(MAX_LEDS);
}

void NullLEDBus::loop() {
}

void NullLEDBus::start(std::unique_lock<std::mutex> &lock, const uint8_t *data,
	size_t length) {
	lock.unlock();
	finish();
}

BackgroundLEDBus::BackgroundLEDBus(const __FlashStringHelper *name) : LEDBus(name) {
}

void BackgroundLEDBus::loop() {
	if (state_ == State::FINISHED) {
		state_ = State::IDLE;
		finish();
	}
}

ByteBufferLEDBus::ByteBufferLEDBus(const __FlashStringHelper *name) : BackgroundLEDBus(name) {
}

void ByteBufferLEDBus::start(std::unique_lock<std::mutex> &lock, const uint8_t *data,
		size_t size) {
	size_t max_bytes = length() * BYTES_PER_LED;

	::memcpy(&buffer_[0], data, std::min(max_bytes, size));

	pos_ = &buffer_[0];
	bytes_ = max_bytes;
	state_ = State::RUNNING;
	transmit();
}

} // namespace aurcor
