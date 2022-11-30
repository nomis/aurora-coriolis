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
#include "led_bus_config.h"
#include "led_profile.h"
#include "led_profiles.h"

namespace aurcor {

class LEDBus {
public:
	static constexpr size_t BYTES_PER_LED = 3;
	static constexpr size_t MAX_BYTES = MAX_LEDS * BYTES_PER_LED;
	static constexpr unsigned long UPDATE_RATE_HZ = 800000;
	static constexpr size_t RESET_TIME_US = 280;
	static constexpr TickType_t SEMAPHORE_TIMEOUT_TICKS = 30 * 1000 * portTICK_PERIOD_MS;

	LEDBus(const char *name, size_t default_length = 1);
	virtual ~LEDBus();

	const char *name() const { return name_; }
	inline size_t length() const { return config_.length(); }
	inline void length(size_t value) { config_.length(value); }
	inline bool reverse() const { return config_.reverse(); }
	inline void reverse(bool value) { config_.reverse(value); }

	inline LEDProfile& profile(enum led_profile_id id) { return profiles_.get(id); }
	inline const LEDProfile& profile(enum led_profile_id id) const { return profiles_.get(id); }
	inline LEDProfile::Result load_profile(enum led_profile_id id) { return profiles_.load(id); }
	inline LEDProfile::Result save_profile(enum led_profile_id id) { return profiles_.save(id); }

	inline uint64_t last_update_us() const { return last_update_us_; }
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

	const char *name_;
	SemaphoreHandle_t semaphore_{nullptr};
	std::atomic<bool> busy_{false};
	uint64_t last_update_us_{0};
	mutable LEDBusConfig config_;
	mutable LEDProfiles profiles_;
};

class NullLEDBus: public LEDBus {
public:
	NullLEDBus(const char *name);
	virtual ~NullLEDBus() = default;

protected:
	void start(const uint8_t *data, size_t size) final override;
};

class ByteBufferLEDBus: public LEDBus {
public:
	ByteBufferLEDBus(const char *name);
	virtual ~ByteBufferLEDBus() = default;

protected:
	void start(const uint8_t *data, size_t size) final override;
	virtual void transmit() = 0;

	std::array<uint8_t,MAX_BYTES> buffer_{};
	const uint8_t *pos_{nullptr};
	size_t bytes_{0};
};

} // namespace aurcor
