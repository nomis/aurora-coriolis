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

#include "aurcor/web_client.h"

#ifdef ENV_NATIVE
#else
# include <Arduino.h>
# include <esp_http_client.h>
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wswitch-enum"
# include <esp_crt_bundle.h>
# pragma GCC diagnostic pop
#endif

#include <memory>
#include <string>

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "web-client";

namespace aurcor {

uuid::log::Logger WebClient::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::FTP};

#ifndef ENV_NATIVE
extern const uint8_t x509_crt_imported_bundle_bin_start[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t x509_crt_imported_bundle_bin_end[]   asm("_binary_x509_crt_bundle_end");
#endif

void WebClient::init() {
#ifndef ENV_NATIVE
	uint16_t num_certs = 0;

	if (x509_crt_imported_bundle_bin_end - x509_crt_imported_bundle_bin_start >= 2) {
		num_certs = (x509_crt_imported_bundle_bin_start[0] << 8) | x509_crt_imported_bundle_bin_start[1];

		esp_crt_bundle_set(x509_crt_imported_bundle_bin_start);
	}

	if (num_certs > 0) {
		logger_.trace(F("Configured %u CA certificates"), num_certs);
	} else {
		logger_.crit(F("No CA certificates"));
	}
#endif
}

bool WebClient::open(const std::string &url) {
#ifdef ENV_NATIVE
	return false;
#else
	esp_err_t err;
	int ret;

	if (handle_) {
		err = esp_http_client_set_url(handle_, url.c_str());
		if (err != ESP_OK) {
			logger_.err(F("URL %s invalid: %d"), url.c_str(), err);
			return false;
		}
	} else {
		esp_http_client_config_t config{};

		config.crt_bundle_attach = esp_crt_bundle_attach;
		config.keep_alive_enable = true;
		config.url = url.c_str();

		handle_ = esp_http_client_init(&config);
		if (!handle_) {
			logger_.err(F("URL %s invalid"), url.c_str());
			return false;
		}
	}

	err = esp_http_client_open(handle_, 0);
	if (err != ESP_OK) {
		logger_.debug(F("GET %s failed: %d"), url.c_str(), err);
		return false;
	}

	ret = esp_http_client_fetch_headers(handle_);
	if (ret < 0) {
		logger_.debug(F("Headers for GET %s failed: %d"), url.c_str(), ret);
		return false;
	}

	ret = esp_http_client_get_status_code(handle_);
	logger_.debug(F("Status code for GET %s: %d"), url.c_str(), ret);
	if (ret != 200)
		return false;

	return true;
#endif
}

ssize_t WebClient::read(char *data, ssize_t size) {
	if (size < 0)
		return -1;

#ifdef ENV_NATIVE
	return -1;
#else
	return esp_http_client_read_response(handle_, data, size);
#endif
}

bool WebClient::done() {
#ifdef ENV_NATIVE
	return false;
#else
	return esp_http_client_is_complete_data_received(handle_);
#endif
}

WebClient::~WebClient() {
#ifndef ENV_NATIVE
	if (handle_)
		esp_http_client_cleanup(handle_);
#endif
}

} // namespace aurcor
