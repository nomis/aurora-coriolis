/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2023  Simon Arlott
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

#ifndef ENV_NATIVE
# include <driver/spi_master.h>
#endif

#include <array>
#include <memory>

#include "led_bus.h"

namespace aurcor {

namespace ledbus {

class SPIPatternTable {
public:
	constexpr SPIPatternTable() {
		for (size_t i = 0; i <= UINT8_MAX; i++) {
			// The LEDs are big-endian but the SPI buffer consists of little-endian bytes and big-endian bits
			values[i] =
				  (((i & (1 << 7)) ? 0b1110 : 0b1000) << (1 * 4))
				| (((i & (1 << 6)) ? 0b1110 : 0b1000) << (0 * 4))
				| (((i & (1 << 5)) ? 0b1110 : 0b1000) << (3 * 4))
				| (((i & (1 << 4)) ? 0b1110 : 0b1000) << (2 * 4))
				| (((i & (1 << 3)) ? 0b1110 : 0b1000) << (5 * 4))
				| (((i & (1 << 2)) ? 0b1110 : 0b1000) << (4 * 4))
				| (((i & (1 << 1)) ? 0b1110 : 0b1000) << (7 * 4))
				| (((i & (1 << 0)) ? 0b1110 : 0b1000) << (6 * 4));
		}
	}

	uint32_t operator[](size_t i) const {
		return values[i];
	}

private:
	std::array<uint32_t,UINT8_MAX + 1> values{};
};

} // namespace ledbus

#ifndef ENV_NATIVE
class SPILEDBus: public LEDBus, public std::enable_shared_from_this<SPILEDBus> {
public:
	SPILEDBus(spi_host_device_t spi_host, const char *name, uint8_t pin);
	~SPILEDBus() override;

	const char *type() const override { return "SPILEDBus"; }

protected:
	void start(const uint8_t *data, size_t size, bool reverse_order) final override;

private:
	class DeviceDeleter {
	public:
		void operator()(spi_device_handle_t handle) {
			spi_bus_remove_device(handle);
		}
	};

	void cleanup();

	static constexpr unsigned long TX_BITS_PER_WORD = sizeof(uint32_t) * 8;
	static constexpr unsigned long TX_WORDS_PER_BYTE = 1;
	static constexpr size_t MAX_WORDS = MAX_LEDS * BYTES_PER_LED * TX_WORDS_PER_BYTE;

	static constexpr unsigned long CLOCK_SPEED_HZ = UPDATE_RATE_HZ * 4;
	static constexpr size_t TX_WORD_US = 1000000 / (CLOCK_SPEED_HZ / TX_BITS_PER_WORD);

	static constexpr size_t MAX_TRANSFER_BYTES = MAX_WORDS * sizeof(uint32_t);

	static IRAM_ATTR void completion_handler(spi_transaction_t *trans);

	spi_host_device_t host_;
	bool host_init_{false};
	std::unique_ptr<struct spi_device_t,DeviceDeleter> device_;
	std::unique_ptr<uint32_t> buffer_;
	spi_transaction_t trans_{};
	uint64_t next_tx_start_us_{0};
	size_t next_tx_delay_us_{0};
	bool ok_;
};
#endif

} // namespace aurcor
