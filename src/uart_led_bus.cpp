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

#include "aurcor/uart_led_bus.h"

#include <Arduino.h>

#ifndef ENV_NATIVE
# include <driver/periph_ctrl.h>
# include <driver/uart.h>
# include <esp_timer.h>
# include <hal/uart_ll.h>
# include <soc/uart_reg.h>
#endif

#include <array>

#include "aurcor/util.h"

namespace aurcor {

namespace ledbus {

static IRAM_ATTR constexpr const UARTPatternTable uart_pattern_table{};

} // namespace ledbus

#ifndef ENV_NATIVE
UARTLEDBus::UARTLEDBus(unsigned int uart_num, const __FlashStringHelper *name,
			uint8_t rx_pin, uint8_t tx_pin) : ByteBufferLEDBus(name),
			periph_(uart_periph_signal[uart_num]),
			hw_(*(UART_LL_GET_HW(uart_num))),
			uart_fifo_reg_(UART_FIFO_REG(uart_num)),
			uart_status_reg_(UART_STATUS_REG(uart_num)) {
	periph_module_enable(periph_.module);

#if SOC_UART_REQUIRE_CORE_RESET
	uart_ll_set_reset_core(&hw_, true);
	periph_module_reset(periph_.module);
	uart_ll_set_reset_core(&hw_, false);
#else
	periph_module_reset(periph_.module);
#endif

	uart_ll_disable_intr_mask(&hw_, UART_LL_INTR_MASK);

	uart_ll_set_sclk(&hw_, UART_SCLK_APB);
	uart_ll_set_baudrate(&hw_, BAUD_RATE);
	uart_ll_set_mode(&hw_, UART_MODE_UART);
	uart_ll_set_parity(&hw_, UART_PARITY_DISABLE);
	uart_ll_set_data_bit_num(&hw_, CFG_UART_WORD_LENGTH);
	uart_ll_set_stop_bits(&hw_, CFG_UART_STOP_BITS);
	uart_ll_set_tx_idle_num(&hw_, 0);
	uart_ll_set_hw_flow_ctrl(&hw_, UART_HW_FLOWCTRL_DISABLE, 0);
	uart_ll_rxfifo_rst(&hw_);
	uart_ll_txfifo_rst(&hw_);

	uart_ll_set_rx_tout(&hw_, 0);
	uart_ll_set_rxfifo_full_thr(&hw_, 0);
	uart_ll_set_txfifo_empty_thr(&hw_, TX_FIFO_THRESHOLD);

	uart_ll_inverse_signal(&hw_, UART_SIGNAL_RXD_INV | UART_SIGNAL_TXD_INV);

	uart_ll_clr_intsts_mask(&hw_, UART_LL_INTR_MASK);

	ok_ = esp_intr_alloc(periph_.irq, ESP_INTR_FLAG_LEVEL1, interrupt_handler,
		this, &interrupt_) == ESP_OK;
	uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	if (ok_) {
		logger_.debug(F("[%S] Configured UART%u on pins RX/%d TX/%d with TX FIFO threshold %zu/%zu"),
			name, uart_num, rx_pin, tx_pin, TX_FIFO_THRESHOLD, TX_FIFO_SIZE);
	} else {
		logger_.emerg(F("[%S] Failed to set up interrupt handler for UART%u"), name, uart_num);
	}
}

UARTLEDBus::~UARTLEDBus() {
	uart_ll_disable_intr_mask(&hw_, UART_LL_INTR_MASK);
	if (ok_)
		esp_intr_free(interrupt_);
	uart_ll_clr_intsts_mask(&hw_, UART_LL_INTR_MASK);

	periph_module_disable(periph_.module);
}

void UARTLEDBus::transmit() {
	uint64_t now;

	while ((now = current_time_us()) < next_tx_start_us_) {
		asm volatile ("nop");
	}

	last_update_us_ = now;

	if (ok_) {
		next_tx_delay_us_ = RESET_TIME_US + std::min(TX_FIFO_MAX_US, TX_BYTE_US * bytes_) + 1;
		uart_ll_ena_intr_mask(&hw_, UART_INTR_TXFIFO_EMPTY);
	} else {
		bytes_ = 0;
		finish();
	}
}

IRAM_ATTR void UARTLEDBus::interrupt_handler(void *arg) {
	auto *self = reinterpret_cast<UARTLEDBus*>(arg);
	auto bytes = self->bytes_;

	if (bytes > 0) {
		const uintptr_t uart_fifo_reg = self->uart_fifo_reg_;
		const uintptr_t uart_status_reg = self->uart_status_reg_;
		auto *pos = self->pos_;

		while (bytes > 0
				&& ((READ_PERI_REG(uart_status_reg) & UART_TXFIFO_CNT_M) >> UART_TXFIFO_CNT_S)
					<= TX_FIFO_SIZE - TX_WORDS_PER_BYTE) {
			int32_t value = ledbus::uart_pattern_table[*pos];

			for (uint8_t i = 0; i < TX_WORDS_PER_BYTE; i++) {
				WRITE_PERI_REG(uart_fifo_reg, value);
				value >>= 8;
			}

			pos++;
			bytes--;
		}

		self->pos_ = pos;
		self->bytes_ = bytes;
	}

	if (bytes == 0) {
		self->next_tx_start_us_ = current_time_us() + self->next_tx_delay_us_;
		uart_ll_disable_intr_mask(&self->hw_, UART_INTR_TXFIFO_EMPTY);
		self->finish_isr();
	} else {
		uart_ll_clr_intsts_mask(&self->hw_, UART_INTR_TXFIFO_EMPTY);
	}
}
#endif

} // namespace aurcor
