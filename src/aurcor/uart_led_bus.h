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

#ifndef ENV_NATIVE
# include <hal/uart_ll.h>
# include <soc/uart_periph.h>
#endif

#include <array>

#include "led_bus.h"

namespace aurcor {

namespace ledbus {

class UARTPatternTable {
public:
	static constexpr unsigned long WORDS_PER_BYTE = 4;

	constexpr UARTPatternTable() {
		for (size_t i = 0; i <= UINT8_MAX; i++) {
			// Writes to the UART FIFO are effectively little-endian
			values[i] =
				  (data[(i >> 6) & 3]      )
				| (data[(i >> 4) & 3] <<  8)
				| (data[(i >> 2) & 3] << 16)
				| (data[(i     ) & 3] << 24);
		}
	}

	uint32_t operator[](size_t i) const {
		return values[i];
	}

private:
	static inline constexpr std::array<uint8_t,WORDS_PER_BYTE> data{
		0b00110111, 0b00000111, 0b00110100, 0b00000100
	};

	std::array<uint32_t,UINT8_MAX + 1> values{};
};

} // namespace ledbus

#ifndef ENV_NATIVE
class UARTLEDBus: public ByteBufferLEDBus {
public:
	UARTLEDBus(unsigned int uart_num, const char *name, uint8_t rx_pin, uint8_t tx_pin);
	~UARTLEDBus() override;

protected:
	void transmit() override;

private:
	static constexpr unsigned long TX_START_BITS = 1;
	static constexpr uart_word_length_t CFG_UART_WORD_LENGTH = UART_DATA_6_BITS;
	static constexpr unsigned long TX_BITS_PER_WORD = 6;
	static constexpr unsigned long TX_WORDS_PER_BYTE = 4;

	static constexpr uart_stop_bits_t CFG_UART_STOP_BITS = UART_STOP_BITS_1;
	static constexpr unsigned long TX_STOP_BITS = 1;

	static constexpr unsigned long BAUD_RATE = UPDATE_RATE_HZ * 4;

	static constexpr uint8_t TX_FIFO_SIZE = UART_LL_FIFO_DEF_LEN;
	static constexpr uint8_t TX_FIFO_MIN_SPACE = ((TX_FIFO_SIZE / 2) / TX_WORDS_PER_BYTE) * TX_WORDS_PER_BYTE;
	static constexpr uint8_t TX_FIFO_THRESHOLD = TX_FIFO_SIZE - TX_FIFO_MIN_SPACE;
	static_assert(TX_FIFO_MIN_SPACE >= TX_WORDS_PER_BYTE,
		"Must be enough space for at least one byte of data when the interrupt is raised");

	static constexpr size_t TX_FIFO_MAX_US = 1000000 /
		(BAUD_RATE / (TX_FIFO_SIZE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));
	static constexpr size_t TX_BYTE_US = 1000000 /
		(BAUD_RATE / (TX_WORDS_PER_BYTE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));

	static_assert(ledbus::UARTPatternTable::WORDS_PER_BYTE == TX_WORDS_PER_BYTE,
		"Pattern table must use the same words per byte");

	static IRAM_ATTR void interrupt_handler(void *arg);

	const uart_signal_conn_t &periph_;
	uart_dev_t &hw_;
	const uintptr_t uart_fifo_reg_;
	const uintptr_t uart_status_reg_;
	uint64_t next_tx_start_us_{0};
	size_t next_tx_delay_us_{0};
	intr_handle_t interrupt_;
	bool ok_;
};
#endif

} // namespace aurcor
