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

#include "aurcor/spi_led_bus.h"

#include <Arduino.h>

#ifndef ENV_NATIVE
# include <driver/spi_master.h>
#endif

#include <array>
#include <cstring>

#include "aurcor/util.h"

namespace aurcor {

namespace ledbus {

static DRAM_ATTR constexpr const SPIPatternTable spi_pattern_table{};

} // namespace ledbus

#ifndef ENV_NATIVE
SPILEDBus::SPILEDBus(spi_host_device_t spi_host, const char *name,
		uint8_t pin) : LEDBus(name), host_(spi_host),
		buffer_(reinterpret_cast<uint32_t*>(::heap_caps_malloc(MAX_TRANSFER_BYTES, MALLOC_CAP_DMA | MALLOC_CAP_8BIT))) {

	if (!buffer_) {
		logger_.err(F("[%S] Unable to allocate %zu bytes for buffer"), name, MAX_TRANSFER_BYTES);
		ok_ = false;
		return;
	}

	spi_bus_config_t bus_config{};
	bus_config.sclk_io_num = -1;
	bus_config.mosi_io_num = pin;
	bus_config.miso_io_num = -1;
	bus_config.data2_io_num = -1;
	bus_config.data3_io_num = -1;
	bus_config.data4_io_num = -1;
	bus_config.data5_io_num = -1;
	bus_config.data6_io_num = -1;
	bus_config.data7_io_num = -1;
	bus_config.max_transfer_sz = MAX_TRANSFER_BYTES;
	bus_config.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_MOSI;
	//bus_config.isr_cpu_id = ARDUINO_RUNNING_CORE;
	bus_config.intr_flags = ESP_INTR_FLAG_LEVEL1;

	spi_device_interface_config_t dev_config{};
	//dev_config.clock_source = SPI_CLK_SRC_DEFAULT;
	dev_config.clock_speed_hz = CLOCK_SPEED_HZ;
	dev_config.spics_io_num = -1;
	dev_config.flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_NO_DUMMY /* | SPI_DEVICE_NO_RETURN_RESULT */;
	dev_config.queue_size = 1;
	dev_config.post_cb = completion_handler;

	esp_err_t err = spi_bus_initialize(host_, &bus_config, SPI_DMA_CH_AUTO);

	if (err) {
		logger_.err(F("[%S] SPI bus error: %d"), name, err);
	} else {
		host_init_ = true;

		spi_device_handle_t handle;

		err = spi_bus_add_device(host_, &dev_config, &handle);

		if (err) {
			logger_.err(F("[%S] SPI device error: %d"), name, err);
		} else {
			device_ = std::unique_ptr<spi_device_t,DeviceDeleter>(handle);
		}
	}

	ok_ = host_init_ && device_;

	trans_.user = this;
	trans_.tx_buffer = buffer_.get();

	if (ok_) {
		logger_.debug(F("[%S] Configured SPI on pin %d"), name, pin);
	} else {
		logger_.emerg(F("[%S] Failed to set up SPI on pin %d"), name, pin);
		cleanup();
	}
}

void SPILEDBus::cleanup() {
	if (host_init_) {
		device_.reset();
		buffer_.reset();
		spi_bus_free(host_);
		host_init_ = false;
	}
}

SPILEDBus::~SPILEDBus() {
	cleanup();
}

void SPILEDBus::start(const uint8_t *data, size_t size, bool reverse_order) {
	if (!ok_) {
		finish();
		return;
	}

	spi_transaction_t *other_trans{nullptr};
	spi_device_get_trans_result(device_.get(), &other_trans, 0);

	const size_t max_bytes = length() * BYTES_PER_LED;

	size /= BYTES_PER_LED;
	size *= BYTES_PER_LED;
	size = std::min(max_bytes, size);

	if (size > 0) {
		if (reverse_order) {
			for (size_t out_bytes = size - BYTES_PER_LED, in_bytes = 0; in_bytes < size;
					out_bytes -= BYTES_PER_LED, in_bytes += BYTES_PER_LED) {
				buffer_.get()[out_bytes] = ledbus::spi_pattern_table[data[in_bytes]];
				buffer_.get()[out_bytes + 1] = ledbus::spi_pattern_table[data[in_bytes + 1]];
				buffer_.get()[out_bytes + 2] = ledbus::spi_pattern_table[data[in_bytes + 2]];
			}
		} else {
			for (size_t i = 0; i < size; i++)
				buffer_.get()[i] = ledbus::spi_pattern_table[data[i]];
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
				buffer_.get()[i] = ledbus::spi_pattern_table[0];
		} else {
			for (size_t i = size; i < max_bytes; i++)
				buffer_.get()[i] = ledbus::spi_pattern_table[0];
		}
	}

	/*
	 * To ensure consistency in the update rate regardless of where the changes
	 * are, we always write everything. (It would be possible to determine where
	 * the last change is in the buffer before overwriting it.)
	 */
	trans_.length = max_bytes * TX_WORDS_PER_BYTE * TX_BITS_PER_WORD;

	next_tx_delay_us_ = reset_time_us() + 1U;

	while (current_time_us() < next_tx_start_us_) {
		asm volatile ("nop");
	}

	esp_err_t err = spi_device_queue_trans(device_.get(), &trans_, 0);
	if (err) {
		finish();
	}
}

IRAM_ATTR void SPILEDBus::completion_handler(spi_transaction_t *trans) {
	auto *self = reinterpret_cast<SPILEDBus*>(trans->user);

	self->next_tx_start_us_ = current_time_us() + self->next_tx_delay_us_;
	self->finish_isr();
}
#endif

} // namespace aurcor
