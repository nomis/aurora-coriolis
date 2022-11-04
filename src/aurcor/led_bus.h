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

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

#include <uuid/log.h>

namespace aurcor {

class LEDBus;

class LEDBus {
public:
	static constexpr size_t MAX_LEDS = 1024;
	static constexpr size_t BYTES_PER_LED = 3;
	static constexpr size_t MAX_BYTES = MAX_LEDS * BYTES_PER_LED;
	static constexpr unsigned long UPDATE_RATE_HZ = 800000;
	static constexpr unsigned long RESET_TIME_US = 280;

	LEDBus(const __FlashStringHelper *name);
	virtual ~LEDBus() = default;

	const __FlashStringHelper *name() const { return name_; }
	inline size_t length() const { return length_; }
	inline void length(size_t length) { length_ = std::min(length, MAX_LEDS); }

	uint64_t last_update_ms() const;

	void write(const uint8_t *data, size_t size); /* data is in RGB order */
	virtual void loop() = 0;

protected:
	virtual void start(std::unique_lock<std::mutex> &lock, const uint8_t *data,
		size_t size) = 0;
	virtual void finish();

	static uuid::log::Logger logger_;

private:
	LEDBus(LEDBus&&) = delete;
	LEDBus(const LEDBus&) = delete;
	LEDBus& operator=(LEDBus&&) = delete;
	LEDBus& operator=(const LEDBus&) = delete;

	const __FlashStringHelper *name_;
	std::atomic<size_t> length_{0};
	mutable std::mutex mutex_;
	std::condition_variable ready_;
	bool busy_{false};
	uint64_t last_update_ms_{0};
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

	void loop() override;

protected:
	void start(std::unique_lock<std::mutex> &lock, const uint8_t *data,
		size_t size) override;
};

class BackgroundLEDBus: public LEDBus {
public:
	BackgroundLEDBus(const __FlashStringHelper *name);
	virtual ~BackgroundLEDBus() = default;

	void loop() override;

protected:
	enum State : uint8_t {
		IDLE = 0,
		RUNNING,
		FINISHED,
	};

	virtual void transmit() = 0;

	std::atomic<State> state_{State::IDLE};
};

class ByteBufferLEDBus: public BackgroundLEDBus {
public:
	ByteBufferLEDBus(const __FlashStringHelper *name);
	virtual ~ByteBufferLEDBus() = default;

protected:
	void start(std::unique_lock<std::mutex> &lock, const uint8_t *data,
		size_t size) override;
	virtual void transmit() = 0;

	std::array<uint8_t,MAX_BYTES> buffer_{};
	const uint8_t *pos_{nullptr};
	size_t bytes_{0};
};

} // namespace aurcor
