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

#include <Arduino.h>

#include <freertos/semphr.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <vector>

#include <uuid/log.h>

#include "constants.h"
#include "led_profile.h"
#include "led_profiles.h"

namespace aurcor {

class LEDBus {
public:
	static constexpr size_t BYTES_PER_LED = 3;
	static constexpr size_t MAX_BYTES = MAX_LEDS * BYTES_PER_LED;
	static constexpr unsigned long UPDATE_RATE_HZ = 800000;
	static constexpr uint64_t RESET_TIME_US = 280;
	static constexpr TickType_t SEMAPHORE_TIMEOUT_TICKS = 30 * 1000 * portTICK_PERIOD_MS;

	LEDBus(const __FlashStringHelper *name);
	virtual ~LEDBus();

	const __FlashStringHelper *name() const { return name_; }
	inline size_t length() const { return length_; }
	inline void length(size_t length) { length_ = std::min(length, MAX_LEDS); }
	inline bool reverse() const { return reverse_; }
	inline void reverse(bool reverse) { reverse_ = reverse; }

	inline LEDProfile& profile(enum led_profile_id id) { return profiles_.get(id); }
	inline const LEDProfile& profile(enum led_profile_id id) const { return profiles_.get(id); }
	inline LEDProfile::Result load_profile(enum led_profile_id id) { return profiles_.load(id); }
	inline LEDProfile::Result save_profile(enum led_profile_id id) { return profiles_.save(id); }

	uint64_t last_update_us() const;
	bool ready() const;
	void write(const uint8_t *data, size_t size); /* data is in RGB order */

protected:
	virtual void start(const uint8_t *data, size_t size) = 0;
	void finish();
	IRAM_ATTR void finish_isr();

	static uuid::log::Logger logger_;

private:
	LEDBus(LEDBus&&) = delete;
	LEDBus(const LEDBus&) = delete;
	LEDBus& operator=(LEDBus&&) = delete;
	LEDBus& operator=(const LEDBus&) = delete;

	const __FlashStringHelper *name_;
	SemaphoreHandle_t semaphore_{nullptr};
	std::atomic<size_t> length_{0};
	std::atomic<bool> reverse_{false};
	std::atomic<bool> busy_{false};
	uint64_t last_update_us_{0};
	mutable LEDProfiles profiles_;
};

namespace ledbus {

static inline void write(LEDBus &bus, const std::vector<uint8_t> data) {
	bus.write(data.data(), data.size());
}

} // namespace ledbus

class NullLEDBus: public LEDBus {
public:
	NullLEDBus(const __FlashStringHelper *name);
	virtual ~NullLEDBus() = default;

protected:
	void start(const uint8_t *data, size_t size) override;
};

class ByteBufferLEDBus: public LEDBus {
public:
	ByteBufferLEDBus(const __FlashStringHelper *name);
	virtual ~ByteBufferLEDBus() = default;

protected:
	void start(const uint8_t *data, size_t size) override;
	virtual void transmit() = 0;

	std::array<uint8_t,MAX_BYTES> buffer_{};
	const uint8_t *pos_{nullptr};
	size_t bytes_{0};
};

} // namespace aurcor
