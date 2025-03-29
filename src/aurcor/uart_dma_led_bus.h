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

#pragma once

#include <Arduino.h>

#ifndef ENV_NATIVE
# include <esp_private/gdma.h>
# include <hal/uart_ll.h>
# include <rom/lldesc.h>
# include <soc/uart_periph.h>
# include <soc/uhci_struct.h>
#endif

#if CONFIG_IDF_TARGET_ESP32H3 || CONFIG_IDF_TARGET_ESP32S3
# define UHCI_USES_GDMA 1
# define SOC_UHCI_NUM 1
#elif CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32H4
# define UHCI_USES_GDMA 0
# define SOC_UHCI_NUM 2
#else
# define UHCI_USES_GDMA 0
# define SOC_UHCI_NUM 1
#endif

#include <array>
#include <memory>

#include "led_bus.h"
#include "uart_led_bus.h"

namespace aurcor {

#ifndef ENV_NATIVE
class UARTDMALEDBus: public LEDBus, public std::enable_shared_from_this<UARTDMALEDBus> {
public:
	UARTDMALEDBus(unsigned int uart_num, uhci_dev_t *uhci_dev, const char *name, uint8_t rx_pin, uint8_t tx_pin);
	~UARTDMALEDBus() override;

	const char *type() const override { return "UARTDMALEDBus"; }

protected:
	void start(const uint8_t *data, size_t size, bool reverse_order) final override;

private:
	static constexpr unsigned long TX_START_BITS = 1;
	static constexpr uart_word_length_t CFG_UART_WORD_LENGTH = UART_DATA_6_BITS;
	static constexpr unsigned long TX_BITS_PER_WORD = 6;
	static constexpr unsigned long TX_WORDS_PER_BYTE = 4;
	static constexpr size_t MAX_TX_BYTES = MAX_LEDS * BYTES_PER_LED * TX_WORDS_PER_BYTE;

	static constexpr size_t MAX_DMA_LENGTH = (1 << 12) - sizeof(uint32_t);
	static constexpr size_t NUM_DMA_DESCS = (MAX_TX_BYTES + MAX_DMA_LENGTH - 1) / MAX_DMA_LENGTH;

	static constexpr uart_stop_bits_t CFG_UART_STOP_BITS = UART_STOP_BITS_1;
	static constexpr unsigned long TX_STOP_BITS = 1;

	static constexpr unsigned long BAUD_RATE = UPDATE_RATE_HZ * 4;

	static constexpr uint8_t TX_FIFO_SIZE = UART_LL_FIFO_DEF_LEN;
	static constexpr uint8_t TX_FIFO_THRESHOLD = TX_FIFO_SIZE - TX_WORDS_PER_BYTE;

	static constexpr size_t TX_FIFO_MAX_US = 1000000 /
		(BAUD_RATE / (TX_FIFO_SIZE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));
	static constexpr size_t TX_BYTE_US = 1000000 /
		(BAUD_RATE / (TX_WORDS_PER_BYTE * (TX_START_BITS + TX_BITS_PER_WORD + TX_STOP_BITS)));

	static_assert(ledbus::UARTPatternTable::WORDS_PER_BYTE == TX_WORDS_PER_BYTE,
		"Pattern table must use the same words per byte");

#if UHCI_USES_GDMA
	static IRAM_ATTR bool completion_handler(gdma_channel_handle_t dma_chan,
		gdma_event_data_t *event_data, void *user_data);
#else
	static IRAM_ATTR void interrupt_handler(void *arg);
#endif

	void cleanup();

	const uart_signal_conn_t &periph_;
	uart_dev_t &hw_;
	uhci_dev_t &uhci_;
	const uintptr_t uart_fifo_reg_;
	const uintptr_t uart_status_reg_;
#if UHCI_USES_GDMA
	gdma_channel_handle_t tx_channel_;
#else
	intr_handle_t interrupt_;
#endif
	std::unique_ptr<lldesc_t> tx_link_;
	std::unique_ptr<uint32_t> buffer_;
	uint64_t next_tx_start_us_{0};
	size_t next_tx_delay_us_{0};
	bool ok_;
};
#endif

} // namespace aurcor
