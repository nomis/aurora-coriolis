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
# include <curl/curl.h>
#else
# include <Arduino.h>
# include <esp_http_client.h>
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wswitch-enum"
# include <esp_crt_bundle.h>
# pragma GCC diagnostic pop
#endif

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

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
#ifdef ENV_NATIVE
	curl_global_init(CURL_GLOBAL_DEFAULT);
	atexit(curl_global_cleanup);
#else
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
	long status_code;
#ifdef ENV_NATIVE
	CURLcode res;

	if (!curl_) {
		curl_ = std::unique_ptr<CURL,CurlDeleter>{curl_easy_init()};

		if (!curl_) {
			logger_.err(F("CURL error"));
			return false;
		}

		res = curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, this);
		if (res != CURLE_OK) {
			logger_.err(F("CURLOPT_WRITEDATA error: %s"), curl_easy_strerror(res));
			curl_.reset();
			return false;
		}

		res = curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, curl_append);
		if (res != CURLE_OK) {
			logger_.err(F("CURLOPT_WRITEDATA error: %s"), curl_easy_strerror(res));
			curl_.reset();
			return false;
		}
	}

	res = curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
	if (res != CURLE_OK) {
		logger_.err(F("URL %s invalid: %s"), curl_easy_strerror(res));
		return false;
	}

	data_.clear();

	res = curl_easy_perform(curl_.get());
	if (res != CURLE_OK) {
		logger_.debug(F("GET %s failed: %s"), url.c_str(),curl_easy_strerror(res));
		return false;
	}

	res = curl_easy_getinfo(curl_.get(), CURLINFO_RESPONSE_CODE, &status_code);
	if (res != CURLE_OK) {
		logger_.debug(F("Status code for GET %s error: %s"), url.c_str(), curl_easy_strerror(res));
		return false;
	}
#else
	esp_err_t err;
	int ret;

	if (handle_) {
		err = esp_http_client_set_url(handle_.get(), url.c_str());
		if (err != ESP_OK) {
			logger_.err(F("URL %s invalid: %d"), url.c_str(), err);
			return false;
		}
	} else {
		esp_http_client_config_t config{};

		config.crt_bundle_attach = esp_crt_bundle_attach;
		config.keep_alive_enable = true;
		config.url = url.c_str();

		handle_ = std::unique_ptr<struct esp_http_client,HandleDeleter>{esp_http_client_init(&config)};
		if (!handle_) {
			logger_.err(F("URL %s invalid"), url.c_str());
			return false;
		}
	}

	err = esp_http_client_open(handle_.get(), 0);
	if (err != ESP_OK) {
		logger_.debug(F("GET %s failed: %d"), url.c_str(), err);
		return false;
	}

	ret = esp_http_client_fetch_headers(handle_.get());
	if (ret < 0) {
		logger_.debug(F("Headers for GET %s failed: %d"), url.c_str(), ret);
		return false;
	}

	status_code = esp_http_client_get_status_code(handle_.get());
#endif

	logger_.debug(F("Status code for GET %s: %ld"), url.c_str(), status_code);
	if (status_code != 200)
		return false;

	return true;
}

#ifdef ENV_NATIVE
size_t WebClient::curl_append(char *ptr, size_t size, size_t nmemb, void *userdata) {
	WebClient *client = reinterpret_cast<WebClient*>(userdata);

	client->data_.insert(client->data_.end(), ptr, ptr + (size * nmemb));

	return size * nmemb;
}
#endif

ssize_t WebClient::read(char *data, ssize_t size) {
	if (size < 0)
		return -1;

#ifdef ENV_NATIVE
	ssize_t available = std::min(size, (ssize_t)data_.size());

	std::memcpy(data, data_.data(), available);
	data_.erase(data_.begin(), std::next(data_.begin(), available));

	return available;
#else
	return esp_http_client_read_response(handle_.get(), data, size);
#endif
}

bool WebClient::done() {
#ifdef ENV_NATIVE
	return data_.empty();
#else
	return esp_http_client_is_complete_data_received(handle_.get());
#endif
}

} // namespace aurcor
