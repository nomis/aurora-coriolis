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
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "aurcor/util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "web-client";

namespace aurcor {

uuid::log::Logger WebClient::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::FTP};

#ifdef ENV_NATIVE
void WebClient::init() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	atexit(curl_global_cleanup);
}
#endif

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

		res = curl_easy_setopt(curl_.get(), CURLOPT_FOLLOWLOCATION, 0);
		if (res != CURLE_OK) {
			logger_.err(F("CURLOPT_FOLLOWLOCATION error: %s"), curl_easy_strerror(res));
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

		config.crt_bundle_attach = arduino_esp_crt_bundle_attach;
		config.keep_alive_enable = true;
		config.disable_auto_redirect = true;
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

	logger_.log(status_code != 200 ? uuid::log::Level::DEBUG : uuid::log::Level::TRACE,
		F("Status code %ld for GET %s"), status_code, url.c_str());
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

	if (available > 0) {
		std::memcpy(data, data_.data(), available);
		data_.erase(data_.begin(), std::next(data_.begin(), available));
	}

	return available;
#else
	return esp_http_client_read_response(handle_.get(), data, size);
#endif
}

std::vector<std::string> WebClient::list_urls(const std::string &url,
		const std::function<bool(const std::string &path)> &filter,
		size_t max_path_length) {
	static constexpr size_t max_tag_length = std::char_traits<char>::length("a");
	static constexpr size_t max_attr_length = std::char_traits<char>::length("href");
	const size_t max_href_len = url.length() + max_path_length;
	std::vector<std::string> urls;
	char buffer[64];

	if (!open(url))
		return {};

	ssize_t len = read(buffer, sizeof(buffer));

	if (len < 0)
		return {};

	enum class State {
		NONE,
		TAG_NAME,
		ATTR_NAME,
		ATTR_EQUALS,
		ATTR_QUOT,
		ATTR_VALUE,
		TAG_WHITESPACE,
	};
	std::string tag_name;
	std::string attr_name;
	std::string attr_value;
	State state = State::NONE;
	char attr_quot = '\0';
	bool a_href_value = false;

	while (len > 0) {
		for (ssize_t i = 0; i < len; i++) {
			char c = buffer[i];

			if (c == '<') {
				state = State::TAG_NAME;
				tag_name.clear();
				continue;
			}

restart:
			switch (state) {
			case State::NONE:
				break;

			case State::TAG_NAME:
				if (c == '/' || c == '>') {
					state = State::NONE;
				} else if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ') {
					if (!tag_name.empty()) {
						if (tag_name == "a") {
							state = State::ATTR_NAME;
							attr_name.clear();
						} else {
							state = State::NONE;
							tag_name.clear();
						}
					}
				} else {
					if (tag_name.size() <= max_tag_length) {
						if (c >= 'A' && c <= 'Z')
							c += ('a' - 'A');

						tag_name.push_back(c);
					}
				}
				break;

			case State::ATTR_NAME:
				if (c == '>') {
					state = State::NONE;
					tag_name.clear();
					attr_name.clear();
				} else if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ') {
					if (!attr_name.empty())
						state = State::ATTR_EQUALS;
				} else if (c == '=') {
					state = State::ATTR_QUOT;
				} else {
					if (attr_name.size() <= max_attr_length) {
						if (c >= 'A' && c <= 'Z')
							c += ('a' - 'A');

						attr_name.push_back(c);
					}
				}
				break;

			case State::ATTR_EQUALS:
				if (c == '>') {
					state = State::NONE;
					tag_name.clear();
					attr_name.clear();
				} else if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ') {
					continue;
				} else if (c == '=') {
					state = State::ATTR_QUOT;
				} else {
					state = State::ATTR_NAME;
					attr_name.clear();
					goto restart;
				}
				break;

			case State::ATTR_QUOT:
				if (c == '>') {
					state = State::NONE;
					tag_name.clear();
					attr_name.clear();
				} else if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ') {
					continue;
				} else {
					state = State::ATTR_VALUE;
					if (c == '"' || c == '\'') {
						attr_quot = c;
					} else {
						attr_quot = ' ';
					}
					if (attr_name == "href") {
						attr_value.clear();
						a_href_value = true;
					} else {
						a_href_value = false;
					}
				}
				break;

			case State::ATTR_VALUE:
				if (c == '>') {
					state = State::NONE;
					tag_name.clear();
					attr_name.clear();
					attr_value.clear();
					continue;
				} else if (a_href_value) {
					if (c == attr_quot) {
						if (attr_value.length() <= max_href_len) {
							std::string absolute_url, relative_url;

							if (normalise_url(url, attr_value, absolute_url, relative_url)) {
								if (!relative_url.empty() && filter(relative_url)) {
									urls.push_back(relative_url);
								}
							}
						}
					} else {
						if (attr_value.length() <= max_href_len)
							attr_value.push_back(c);

						continue;
					}
				}

				if (c == attr_quot) {
					if (attr_quot == ' ') {
						state = State::TAG_WHITESPACE;
					} else {
						state = State::ATTR_NAME;
						attr_name.clear();
					}
				}
				break;

			case State::TAG_WHITESPACE:
				if (c == '>') {
					state = State::NONE;
				} else if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ') {
					state = State::ATTR_NAME;
					attr_name.clear();
				}
				break;
			}
		}

		len = read(buffer, sizeof(buffer));
		if (len < 0)
			return {};
	}

	return urls;
}

static void url_to_lowercase(std::string &url) {
	for (size_t slashes = 0, i = 0; i < url.length(); i++) {
		if (url[i] == '/') {
			slashes++;
			if (slashes == 3)
				break;
		} else if (url[i] >= 'A' && url[i] <= 'Z') {
			url[i] += ('a' - 'A');
		}
	}
}

bool WebClient::normalise_url(std::string base, std::string url,
		std::string &absolute_url, std::string &relative_url) {
	url_to_lowercase(base);

	size_t scheme_pos = base.find("://");

	if (scheme_pos == std::string::npos)
		return false;

	std::string scheme = base.substr(0, scheme_pos);

	if (scheme != "http" && scheme != "https")
		return false;

	size_t path_pos = base.find("/", scheme_pos + std::char_traits<char>::length("://") + 1);

	if (path_pos == std::string::npos) {
		path_pos = base.length();
		base.push_back('/');
	}

	if (url.rfind("//", 0) != std::string::npos) {
		url_to_lowercase(url);
		absolute_url = scheme + ":" + url;
	} else if (url[0] == '/') {
		absolute_url = base.substr(0, path_pos) + url;
	} else if (str_begins_case_insensitive(url, "http://")
			|| str_begins_case_insensitive(url, "https://")) {
		url_to_lowercase(url);
	} else {
		absolute_url = base + url;
	}

	if (path_pos != base.length() - 1
			&& absolute_url.rfind(base, 0) != std::string::npos) {
		relative_url = absolute_url.substr(base.length());
	} else {
		relative_url = "";
	}

	return true;
}

} // namespace aurcor
