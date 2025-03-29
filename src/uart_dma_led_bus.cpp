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

#include "aurcor/uart_dma_led_bus.h"

#include <Arduino.h>

#ifndef ENV_NATIVE
# include <driver/periph_ctrl.h>
# include <driver/uart.h>
# include <esp_private/gdma.h>
# include <esp_timer.h>
# include <hal/uart_ll.h>
# include <rom/lldesc.h>
# include <soc/periph_defs.h>
# include <soc/uart_reg.h>
# include <soc/uhci_reg.h>
# include <soc/uhci_struct.h>
#endif

#include <array>
#include <cstring>

#include "aurcor/util.h"

namespace aurcor {

#ifndef ENV_NATIVE
UARTDMALEDBus::UARTDMALEDBus(unsigned int uart_num, uhci_dev_t *uhci_dev,
			const char *name, uint8_t rx_pin, uint8_t tx_pin)
			: LEDBus(name), periph_(uart_periph_signal[uart_num]),
			hw_(*(UART_LL_GET_HW(uart_num))),
			uhci_(*uhci_dev),
			uart_fifo_reg_(UART_FIFO_REG(uart_num)),
			uart_status_reg_(UART_STATUS_REG(uart_num)),
			tx_link_(reinterpret_cast<lldesc_t*>(::heap_caps_malloc(sizeof(lldesc_t) * NUM_DMA_DESCS, MALLOC_CAP_DMA | MALLOC_CAP_8BIT))),
			buffer_(reinterpret_cast<uint32_t*>(::heap_caps_malloc(MAX_TX_BYTES, MALLOC_CAP_DMA | MALLOC_CAP_8BIT))) {

	if (!tx_link_) {
		logger_.err(F("[%S] Unable to allocate %zu bytes for DMA descriptors"), name, sizeof(lldesc_t) * NUM_DMA_DESCS);
		ok_ = false;
		cleanup();
		return;
	}

	if (!buffer_) {
		logger_.err(F("[%S] Unable to allocate %zu bytes for buffer"), name, MAX_TX_BYTES);
		ok_ = false;
		cleanup();
		return;
	}

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

	uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	if (&uhci_ == &UHCI0) {
		periph_module_enable(PERIPH_UHCI0_MODULE);
		periph_module_reset(PERIPH_UHCI0_MODULE);
#if SOC_UHCI_NUM > 1
	} else if (&uhci_ == &UHCI1) {
		periph_module_enable(PERIPH_UHCI1_MODULE);
		periph_module_reset(PERIPH_UHCI1_MODULE);
#endif
	} else {
		ok_ = false;
		cleanup();
		return;
	}
	uhci_.int_ena.val = 0;
	uhci_.conf0.val = 0;

#if UHCI_USES_GDMA
	uhci_.conf0.rx_rst = 1;
	uhci_.conf0.rx_rst = 0;
	uhci_.conf0.tx_rst = 1;
	uhci_.conf0.tx_rst = 0;
#else
	uhci_.conf0.in_rst = 1;
	uhci_.conf0.in_rst = 0;
	uhci_.conf0.out_rst = 1;
	uhci_.conf0.out_rst = 0;
	uhci_.conf0.out_auto_wrback = 1;
#endif
	uhci_.conf1.check_seq_en = 0;
	uhci_.conf1.check_sum_en = 0;
	uhci_.conf1.tx_ack_num_re = 0;
	uhci_.conf1.tx_check_sum_re = 0;
#if !UHCI_USES_GDMA
	uhci_.conf1.check_owner = 1;
#endif
	uhci_.escape_conf.val = 0;
	if (uart_num == 0) {
		uhci_.conf0.uart0_ce = 1;
#if SOC_UART_NUM > 1
	} else if (uart_num == 1) {
		uhci_.conf0.uart1_ce = 1;
#endif
#if SOC_UART_NUM > 2
	} else if (uart_num == 2) {
		uhci_.conf0.uart2_ce = 1;
#endif
	} else {
		ok_ = false;
		cleanup();
		return;
	}
	uhci_.int_clr.val = uhci_.int_st.val;
#if UHCI_USES_GDMA
	uhci_.int_ena.val = 0;
#else
	uhci_.int_ena.val = UHCI_OUT_TOTAL_EOF_INT_ENA | UHCI_OUT_DSCR_ERR_INT_ENA;
#endif

	std::memset(tx_link_.get(), 0, sizeof(lldesc_t) * NUM_DMA_DESCS);

	lldesc_t *tx_link = tx_link_.get();
	size_t offset = 0;
	size_t remaining = MAX_TX_BYTES;

	for (size_t i = 0; i < NUM_DMA_DESCS; i++) {
		tx_link[i].size = std::min(MAX_DMA_LENGTH, remaining);
		tx_link[i].offset = 0;
		tx_link[i].length = 0;
		tx_link[i].sosf = 0;
		tx_link[i].buf = reinterpret_cast<uint8_t*>(buffer_.get()) + offset;
		if (i + 1 < NUM_DMA_DESCS) {
			tx_link[i].eof = 0;
			tx_link[i].qe.stqe_next = &tx_link[i + 1];
		} else {
			tx_link[i].eof = 1;
			tx_link[i].qe.stqe_next = nullptr;
		}
		tx_link[i].owner = 0;

		offset += tx_link[i].size;
		remaining -= tx_link[i].size;
	}

#if UHCI_USES_GDMA
	esp_err_t err;

	gdma_channel_alloc_config_t channel_config{};
	channel_config.direction = GDMA_CHANNEL_DIRECTION_TX;

	err = gdma_new_channel(&channel_config, &tx_channel_);
	if (err != ESP_OK) {
		logger_.emerg(F("[%S] Failed to create DMA channel for UART%u: %d"), name, uart_num, err);
		ok_ = false;
		cleanup();
		return;
	}

	if (&uhci_ == &UHCI0) {
		err = gdma_connect(tx_channel_, GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_UART, 0));
#if SOC_UHCI_NUM > 1
	} else if (&uhci_ == &UHCI1) {
		err = gdma_connect(tx_channel_, GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_UART, 1));
#endif
	}

	if (err != ESP_OK) {
		logger_.emerg(F("[%S] Failed to connect DMA channel for UART%u: %d"), name, uart_num, err);
		ok_ = false;
		gdma_del_channel(tx_channel_);
		cleanup();
		return;
	}

	gdma_strategy_config_t strategy_config{};
	strategy_config.auto_update_desc = true;
	strategy_config.owner_check = true;

	err = gdma_apply_strategy(tx_channel_, &strategy_config);
	if (err != ESP_OK) {
		logger_.emerg(F("[%S] Failed to configure DMA channel for UART%u: %d"), name, uart_num, err);
		ok_ = false;
		gdma_disconnect(tx_channel_);
		gdma_del_channel(tx_channel_);
		cleanup();
		return;
	}

	ok_ = true;

	gdma_tx_event_callbacks_t callbacks{};
	callbacks.on_trans_eof = completion_handler;
	err = gdma_register_tx_event_callbacks(tx_channel_, &callbacks, this);

	if (err == ESP_OK) {
		logger_.debug(F("[%S] Configured UART%u on pins RX/%d TX/%d at %ubps"),
			name, uart_num, rx_pin, tx_pin, uart_ll_get_baudrate(&hw_));
	} else {
		logger_.emerg(F("[%S] Failed to register DMA callbacks for UART%u: %d"), name, uart_num, err);
		cleanup();
	}
# else
	uhci_.dma_out_link.addr = (uintptr_t)&tx_link[0];

	if (&uhci_ == &UHCI0) {
		ok_ = esp_intr_alloc(ETS_UHCI0_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1,
			interrupt_handler, this, &interrupt_) == ESP_OK;
# if SOC_UHCI_NUM > 1
	} else if (&uhci_ == &UHCI1) {
		ok_ = esp_intr_alloc(ETS_UHCI1_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1,
			interrupt_handler, this, &interrupt_) == ESP_OK;
# endif
	} else {
		ok_ = false;
	}

	if (ok_) {
		logger_.debug(F("[%S] Configured UART%u on pins RX/%d TX/%d at %ubps on CPU%d"),
			name, uart_num, rx_pin, tx_pin, uart_ll_get_baudrate(&hw_), esp_intr_get_cpu(interrupt_));
	} else {
		logger_.emerg(F("[%S] Failed to set up interrupt handler for UART%u"), name, uart_num);
		cleanup();
	}
#endif
}

void UARTDMALEDBus::cleanup() {
	uhci_.int_ena.val = 0;
	if (ok_) {
#if UHCI_USES_GDMA
		gdma_disconnect(tx_channel_);
		gdma_del_channel(tx_channel_);
#else
		esp_intr_free(interrupt_);
#endif
		ok_ = false;
	}
	uhci_.int_clr.val = uhci_.int_st.val;

	if (&uhci_ == &UHCI0) {
		periph_module_disable(PERIPH_UHCI0_MODULE);
#if SOC_UHCI_NUM > 1
	} else if (&uhci_ == &UHCI1) {
		periph_module_disable(PERIPH_UHCI1_MODULE);
#endif
	}
	periph_module_disable(periph_.module);

	buffer_.reset();
	tx_link_.reset();
}

UARTDMALEDBus::~UARTDMALEDBus() {
	cleanup();
}

void UARTDMALEDBus::start(const uint8_t *data, size_t size, bool reverse_order) {
	if (!ok_) {
		finish();
		return;
	}

	const size_t max_bytes = length() * BYTES_PER_LED;
	uint32_t *buffer = buffer_.get();
	lldesc_t *tx_link = tx_link_.get();

	size /= BYTES_PER_LED;
	size *= BYTES_PER_LED;
	size = std::min(max_bytes, size);

	if (size > 0) {
		if (reverse_order) {
			for (size_t out_bytes = size - BYTES_PER_LED, in_bytes = 0; in_bytes < size;
					out_bytes -= BYTES_PER_LED, in_bytes += BYTES_PER_LED) {
				buffer[out_bytes] = ledbus::uart_pattern_table[data[in_bytes]];
				buffer[out_bytes + 1] = ledbus::uart_pattern_table[data[in_bytes + 1]];
				buffer[out_bytes + 2] = ledbus::uart_pattern_table[data[in_bytes + 2]];
			}
		} else {
			for (size_t i = 0; i < size; i++)
				buffer[i] = ledbus::uart_pattern_table[data[i]];
		}
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
			for (size_t i = 0; i < max_bytes - size; i++)
				buffer[i] = ledbus::uart_pattern_table[0];
		} else {
			for (size_t i = size; i < max_bytes; i++)
				buffer[i] = ledbus::uart_pattern_table[0];
		}
	}

	/*
	 * To ensure consistency in the update rate regardless of where the changes
	 * are, we always write everything. (It would be possible to determine where
	 * the last change is in the buffer before overwriting it.)
	 */
	size_t offset = 0;
	size_t remaining = max_bytes * TX_WORDS_PER_BYTE;

	for (size_t i = 0; i < NUM_DMA_DESCS && remaining > 0; i++) {
		if (tx_link[i].owner != 0) {
			logger_.emerg(F("[%S] DMA descriptor %u still owned by hardware"), name(), i);
			cleanup();
			finish();
			return;
		} else {
			tx_link[i].length = std::min(MAX_DMA_LENGTH, remaining);

			offset += tx_link[i].length;
			remaining -= tx_link[i].length;

			if (remaining > 0 && i + 1 < NUM_DMA_DESCS) {
				tx_link[i].eof = 0;
				tx_link[i].qe.stqe_next = &tx_link[i + 1];
			} else {
				tx_link[i].eof = 1;
				tx_link[i].qe.stqe_next = nullptr;
			}
			tx_link[i].owner = 1;
		}
	}

	next_tx_delay_us_ = reset_time_us() + std::min(TX_FIFO_MAX_US, TX_BYTE_US * max_bytes) + 1U;

	while (current_time_us() < next_tx_start_us_) {
		asm volatile ("nop");
	}

	tx_link_->owner = 1;
#if UHCI_USES_GDMA
	esp_err_t err = gdma_start(tx_channel_, (intptr_t)&tx_link[0]);
	if (err != ESP_OK) {
		logger_.emerg(F("[%S] DMA start failed: %d"), name(), err);
		cleanup();
		finish();
	}
#else
	uhci_.dma_out_link.start = 1;
#endif
}

#if UHCI_USES_GDMA
IRAM_ATTR bool UARTDMALEDBus::completion_handler(gdma_channel_handle_t dma_chan,
		gdma_event_data_t *event_data, void *user_data) {
	auto *self = reinterpret_cast<UARTDMALEDBus*>(user_data);

	self->next_tx_start_us_ = current_time_us() + self->next_tx_delay_us_;
	self->finish_isr();

	return false;
}
#else
IRAM_ATTR void UARTDMALEDBus::interrupt_handler(void *arg) {
	auto *self = reinterpret_cast<UARTDMALEDBus*>(arg);
	auto &uhci = self->uhci_;
	uint32_t status = uhci.int_st.val;

	uhci.int_clr.val = status;

	if (status & UHCI_OUT_TOTAL_EOF_INT_ST) {
		self->next_tx_start_us_ = current_time_us() + self->next_tx_delay_us_;
		self->finish_isr();
	} else if (status & UHCI_OUT_DSCR_ERR_INT_ST) {
		self->tx_link_->owner = 0;
		self->next_tx_start_us_ = 0;
		self->finish_isr();
	}
}
#endif

#endif

} // namespace aurcor
