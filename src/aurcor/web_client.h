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

#ifdef ENV_NATIVE
# include <curl/curl.h>
#else
# include <Arduino.h>
# include <esp_http_client.h>
#endif

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <uuid/log.h>

namespace aurcor {

class WebClient {
public:
	static bool normalise_url(std::string base, std::string url,
		std::string &absolute_url, std::string &relative_url);

	WebClient() = default;

#ifdef ENV_NATIVE
	static void init();
#endif

	bool open(const std::string &url);
	ssize_t read(char *data, ssize_t size);

	std::vector<std::string> list_urls(const std::string &url,
		const std::function<bool(const std::string &path)> &filter,
		size_t max_path_length = 64);

private:
#ifdef ENV_NATIVE
	class CurlDeleter {
	public:
		void operator()(CURL *curl) { curl_easy_cleanup(curl); }
	};
#else
	class HandleDeleter {
	public:
		void operator()(esp_http_client_handle_t handle) {
			esp_http_client_cleanup(handle);
		}
	};
#endif

	static size_t curl_append(char *ptr, size_t size, size_t nmemb, void *userdata);

	static uuid::log::Logger logger_;

#ifdef ENV_NATIVE
	std::unique_ptr<CURL,CurlDeleter> curl_;
	std::vector<char> data_;
#else
	std::unique_ptr<struct esp_http_client,HandleDeleter> handle_;
#endif
};

} // namespace aurcor
