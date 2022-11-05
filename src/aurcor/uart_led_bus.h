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
# include <driver/periph_ctrl.h>
# include <driver/uart.h>
# include <esp_timer.h>
# include <hal/uart_ll.h>
# include <soc/uart_reg.h>
#endif

#include <array>

#include "led_bus.h"

namespace aurcor {

namespace ledbus {

class UARTPatternTable {
public:
	static constexpr unsigned long TX_WORDS_PER_BYTE = 4;

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
	static inline constexpr std::array<uint8_t,TX_WORDS_PER_BYTE> data{
		0b00110111, 0b00000111, 0b00110100, 0b00000100
	};

	std::array<uint32_t,UINT8_MAX + 1> values{};
};

static IRAM_ATTR constexpr const UARTPatternTable uart_pattern_table{};

} // namespace ledbus

#ifndef ENV_NATIVE
template<unsigned int UARTNumber>
class UARTLEDBus: public ByteBufferLEDBus {
public:
	UARTLEDBus(const __FlashStringHelper *name, uint8_t rx_pin,
			uint8_t tx_pin) : ByteBufferLEDBus(name) {
		periph_module_enable(periph.module);

#if SOC_UART_REQUIRE_CORE_RESET
		uart_ll_set_reset_core(&hw, true);
		periph_module_reset(periph.module);
		uart_ll_set_reset_core(&hw, false);
#else
		periph_module_reset(periph.module);
#endif

		uart_ll_disable_intr_mask(&hw, UART_LL_INTR_MASK);

		uart_ll_set_sclk(&hw, UART_SCLK_APB);
		uart_ll_set_baudrate(&hw, BAUD_RATE);
		uart_ll_set_mode(&hw, UART_MODE_UART);
		uart_ll_set_parity(&hw, UART_PARITY_DISABLE);
		uart_ll_set_data_bit_num(&hw, CFG_UART_WORD_LENGTH);
		uart_ll_set_stop_bits(&hw, CFG_UART_STOP_BITS);
		uart_ll_set_tx_idle_num(&hw, 0);
		uart_ll_set_hw_flow_ctrl(&hw, UART_HW_FLOWCTRL_DISABLE, 0);
		uart_ll_rxfifo_rst(&hw);
		uart_ll_txfifo_rst(&hw);

		uart_ll_set_rx_tout(&hw, 0);
		uart_ll_set_rxfifo_full_thr(&hw, 0);
		uart_ll_set_txfifo_empty_thr(&hw, TX_FIFO_THRESHOLD);

		uart_ll_inverse_signal(&hw, UART_SIGNAL_RXD_INV | UART_SIGNAL_TXD_INV);

		uart_ll_clr_intsts_mask(&hw, UART_LL_INTR_MASK);

		ok_ = esp_intr_alloc(periph.irq, ESP_INTR_FLAG_LEVEL1, interrupt_handler,
			this, &interrupt_) == ESP_OK;
		uart_set_pin(UARTNumber, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

		if (ok_) {
			logger_.debug(F("[%S] Configured UART%u on pins RX/%d TX/%d with TX FIFO threshold %zu/%zu"),
				name, UARTNumber, rx_pin, tx_pin, TX_FIFO_THRESHOLD, TX_FIFO_SIZE);
		} else {
			logger_.emerg(F("[%S] Failed to set up interrupt handler for UART%u"), name, UARTNumber);
		}
	}

	~UARTLEDBus() override {
		uart_ll_disable_intr_mask(&hw, UART_LL_INTR_MASK);
		if (ok_)
			esp_intr_free(interrupt_);
		uart_ll_clr_intsts_mask(&hw, UART_LL_INTR_MASK);

		periph_module_disable(periph.module);
	}

protected:
	void transmit() override {
		while ((uint64_t)esp_timer_get_time() < next_tx_start_us_) {
			asm volatile ("nop");
		}

		if (ok_) {
			next_tx_delay_us_ = RESET_TIME_US + std::min(TX_FIFO_MAX_US, TX_BYTE_US * bytes_) + 1;
			uart_ll_ena_intr_mask(&hw, UART_INTR_TXFIFO_EMPTY);
		} else {
			bytes_ = 0;
			finish();
		}
	}

private:
	static constexpr const uart_signal_conn_t &periph = uart_periph_signal[UARTNumber];
	static constexpr uart_dev_t &hw = *(UART_LL_GET_HW(UARTNumber));

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

	static constexpr uint64_t TX_FIFO_MAX_US = 1000000 /
		(BAUD_RATE / (TX_FIFO_SIZE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));
	static constexpr uint64_t TX_BYTE_US = 1000000 /
		(BAUD_RATE / (TX_WORDS_PER_BYTE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));

	static_assert(ledbus::uart_pattern_table.TX_WORDS_PER_BYTE == TX_WORDS_PER_BYTE,
		"Pattern table must use the same words per byte");

	static IRAM_ATTR void interrupt_handler(void *arg) {
		auto *self = reinterpret_cast<UARTLEDBus<UARTNumber>*>(arg);
		auto bytes = self->bytes_;

		if (bytes > 0) {
			auto *pos = self->pos_;

			while (bytes > 0 && uart_ll_get_txfifo_len(&hw) >= TX_WORDS_PER_BYTE) {
				int32_t value = ledbus::uart_pattern_table[*pos];

				for (uint8_t i = 0; i < TX_WORDS_PER_BYTE; i++) {
					WRITE_PERI_REG(UART_FIFO_REG(UARTNumber), value);
					value >>= 8;
				}

				pos++;
				bytes--;
			}

			self->pos_ = pos;
			self->bytes_ = bytes;
		}

		if (bytes == 0) {
			self->next_tx_start_us_ = esp_timer_get_time() + self->next_tx_delay_us_;
			uart_ll_disable_intr_mask(&hw, UART_INTR_TXFIFO_EMPTY);
			self->finish_isr();
		} else {
			uart_ll_clr_intsts_mask(&hw, UART_INTR_TXFIFO_EMPTY);
		}
	}

	uint64_t next_tx_start_us_{0};
	uint32_t next_tx_delay_us_{0};
	intr_handle_t interrupt_;
	bool ok_;
};
#endif

} // namespace aurcor
