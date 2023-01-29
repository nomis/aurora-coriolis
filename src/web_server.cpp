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

#include "aurcor/web_server.h"

#include <Arduino.h>

#ifdef ENV_NATIVE
# include <sys/types.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <microhttpd.h>
#else
# include <esp_http_server.h>
#endif

#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "web-server";

namespace aurcor {

uuid::log::Logger WebServer::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

WebServer::WebServer(uint16_t port) {
#ifdef ENV_NATIVE
	struct MHD_Daemon *daemon;
	const char *port_env = std::getenv("HTTPD_PORT");

	if (port_env)
		port = std::strtol(port_env, nullptr, 0);

	daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, nullptr,
		nullptr, &handle_connection, this,
		MHD_OPTION_NOTIFY_COMPLETED, &cleanup_connection, nullptr,
		MHD_OPTION_URI_LOG_CALLBACK, &log_connection, nullptr,
		MHD_OPTION_END);

	if (daemon) {
		daemon_ = std::unique_ptr<struct MHD_Daemon,MHD_DaemonDeleter>{daemon};
		logger_.debug("Started HTTP server: http://localhost:%u",
			MHD_get_daemon_info(daemon_.get(), MHD_DAEMON_INFO_BIND_PORT)->port);
	} else {
		logger_.crit("Failed to start HTTP server");
	}
#else
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	httpd_handle_t server = nullptr;
	esp_err_t err;

	config.core_id = ARDUINO_RUNNING_CORE;
	config.task_priority = uxTaskPriorityGet(nullptr);
	config.server_port = port;

	err = httpd_start(&server, &config);

	if (err == ESP_OK) {
		handle_ = std::unique_ptr<void,HandleDeleter>{server};
		logger_.debug("Started HTTP server");
	} else {
		logger_.crit("Failed to start HTTP server: %d", err);
	}
#endif
}

WebServer::~WebServer() {
#ifdef ENV_NATIVE
#else
	if (!handle_)
		return;

	for (auto &uri_handler : uri_handlers_)
		uri_handler->server_unregister(handle_.get());
#endif
}

bool WebServer::add_get_handler(const std::string &uri, get_function handler) {
#ifdef ENV_NATIVE
	if (!daemon_)
		return false;
#else
	if (!handle_)
		return false;
#endif

	uri_handlers_.push_back(std::make_unique<GetURIHandler>(uri, std::move(handler)));

#ifdef ENV_NATIVE
	return true;
#else
	if (uri_handlers_.back()->server_register(handle_.get()))
		return true;

	logger_.crit("Failed to register GET handler for URI %s", uri.c_str());

	uri_handlers_.pop_back();
	return false;
#endif
}

bool WebServer::add_post_handler(const std::string &uri, get_function handler) {
#ifdef ENV_NATIVE
	if (!daemon_)
		return false;
#else
	if (!handle_)
		return false;
#endif

	uri_handlers_.push_back(std::make_unique<PostURIHandler>(uri, std::move(handler)));

#ifdef ENV_NATIVE
	return true;
#else
	if (uri_handlers_.back()->server_register(handle_.get()))
		return true;

	logger_.crit("Failed to register POST handler for URI %s", uri.c_str());

	uri_handlers_.pop_back();
	return false;
#endif
}

bool WebServer::add_static_content(const std::string &uri, const char *content_type,
		const char * const headers[][2], const std::string_view data) {
#ifdef ENV_NATIVE
	if (!daemon_)
		return false;
#else
	if (!handle_)
		return false;
#endif

	uri_handlers_.push_back(std::make_unique<StaticContentURIHandler>(uri,
		content_type, headers, data));

#ifdef ENV_NATIVE
	return true;
#else
	if (uri_handlers_.back()->server_register(handle_.get()))
		return true;

	logger_.crit("Failed to register GET handler for URI %s", uri.c_str());

	uri_handlers_.pop_back();
	return false;
#endif
}

WebServer::URIHandler::URIHandler(const std::string &uri) : uri_(uri) {
}

#ifdef ENV_NATIVE
#else
bool WebServer::URIHandler::server_register(httpd_handle_t server) {
	httpd_uri_t httpd_handler;

	httpd_handler.method = method();
	httpd_handler.uri = uri_.c_str();
	httpd_handler.user_ctx = this;
	httpd_handler.handler = [] (httpd_req_t *req) -> esp_err_t {
		return reinterpret_cast<WebServer::URIHandler*>(req->user_ctx)->handler_function(req);
	};

	return httpd_register_uri_handler(server, &httpd_handler) == ESP_OK;
}

void WebServer::URIHandler::server_unregister(httpd_handle_t server) {
	httpd_unregister_uri_handler(server, uri_.c_str(), method());
}
#endif

WebServer::GetURIHandler::GetURIHandler(const std::string &uri, get_function handler)
		: URIHandler(uri), function_(handler) {
}

WebServer::PostURIHandler::PostURIHandler(const std::string &uri, post_function handler)
		: URIHandler(uri), function_(handler) {
}

WebServer::StaticContentURIHandler::StaticContentURIHandler(const std::string &uri,
		const char *content_type, const char * const headers[][2],
		const std::string_view data)
		: URIHandler(uri), content_type_(content_type), headers_(headers),
		data_(data) {
}

#ifdef ENV_NATIVE
std::string WebServer::GetURIHandler::method() {
	return "GET";
}

int WebServer::GetURIHandler::handle_connection(Request &req) {
	if (function_(req)) {
		return req.finish();
	} else {
		return MHD_NO;
	}
}

std::string WebServer::PostURIHandler::method() {
	return "POST";
}

int WebServer::PostURIHandler::handle_connection(Request &req) {
	if (function_(req)) {
		return req.finish();
	} else {
		return MHD_NO;
	}
}

std::string WebServer::StaticContentURIHandler::method() {
	return "GET";
}

int WebServer::StaticContentURIHandler::handle_connection(Request &req) {
	req.set_status(200);
	req.set_type(content_type_);

	if (headers_ != nullptr) {
		const char * const *header = headers_[0];

		while (header[0] != nullptr) {
			req.add_header(header[0], header[1]);
			header += 2;
		}
	}

	req.write(reinterpret_cast<const uint8_t*>(data_.begin()), data_.length());
	return req.finish();
}
#else
httpd_method_t WebServer::GetURIHandler::method() {
	return HTTP_GET;
}

esp_err_t WebServer::GetURIHandler::handler_function(httpd_req_t *req) {
	Request ws_req{req};

	if (function_(ws_req)) {
		ws_req.finish();
		return ESP_OK;
	} else {
		return ESP_FAIL;
	}
}

httpd_method_t WebServer::PostURIHandler::method() {
	return HTTP_POST;
}

esp_err_t WebServer::PostURIHandler::handler_function(httpd_req_t *req) {
	Request ws_req{req};

	if (function_(ws_req)) {
		ws_req.finish();
		return ESP_OK;
	} else {
		return ESP_FAIL;
	}
}

httpd_method_t WebServer::StaticContentURIHandler::method() {
	return HTTP_GET;
}

esp_err_t WebServer::StaticContentURIHandler::handler_function(httpd_req_t *req) {
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, content_type_);

	if (headers_ != nullptr) {
		const char * const *header = headers_[0];

		while (header[0] != nullptr) {
			httpd_resp_set_hdr(req, header[0], header[1]);
			header += 2;
		}
	}

	return httpd_resp_send(req, data_.begin(), data_.length());
}
#endif

#ifdef ENV_NATIVE
WebServer::Request::Request(struct MHD_Connection *connection, const char *url)
		: connection_(connection), url_(url) {
}
#else
WebServer::Request::Request(httpd_req_t *req) : req_(req),
	content_len_(req_->content_len), buffer_(1436 - 7) {
}
#endif

#ifdef ENV_NATIVE
bool WebServer::Request::first() {
	if (first_) {
		first_ = false;
		return true;
	} else {
		return false;
	}
}

void WebServer::Request::upload(const char *data, size_t len) {
	upload_data_.resize(upload_data_.size() + len);
	std::memcpy(&upload_data_[upload_data_.size() - len], data, len);
}

int WebServer::Request::available() {
	return upload_data_.size();
}

int WebServer::Request::read() {
	char buffer;

	if (readBytes(&buffer, 1) == 1) {
		return buffer;
	} else {
		return -1;
	}
}

int WebServer::Request::peek() {
	return -1;
}

size_t WebServer::Request::readBytes(char *buffer, size_t length) {
	length = std::min(length, upload_data_.size());
	if (length > 0) {
		std::memcpy(buffer, upload_data_.data(), length);
		upload_data_.erase(upload_data_.begin(), std::next(upload_data_.begin(), length));
	}
	return length;
}

size_t WebServer::Request::write(uint8_t c) {
	buffer_.push_back(c);
	return 1;
}

size_t WebServer::Request::write(const uint8_t *buffer, size_t size) {
	buffer_.resize(buffer_.size() + size);
	std::memcpy(&buffer_[buffer_.size() - size], buffer, size);
	return size;
}

int WebServer::Request::finish() {
	if (status_ == 0)
		status_ = buffer_.empty() ? 204 : 200;

	auto response = wrap_response(MHD_create_response_from_buffer(
			buffer_.size(), reinterpret_cast<void*>(buffer_.data()),
			MHD_RESPMEM_MUST_COPY));
	auto ret = MHD_queue_response(connection_, status_, response.get());

	MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONTENT_TYPE, content_type_);
	for (auto &header : resp_headers_)
		MHD_add_response_header(response.get(), header.first.c_str(), header.second.c_str());

	return ret;
}
#else
int WebServer::Request::available() {
	return content_len_;
}

int WebServer::Request::read() {
	char buffer;

	if (content_len_ > 0 && readBytes(&buffer, 1) == 1) {
		return buffer;
	} else {
		return -1;
	}
}

int WebServer::Request::peek() {
	return -1;
}

size_t WebServer::Request::readBytes(char *buffer, size_t length) {
	esp_err_t ret = httpd_req_recv(req_, buffer, length);
	return ret < 0 ? 0 : ret;
}

size_t WebServer::Request::write(uint8_t c) {
	buffer_[buffer_len_] = (char)c;
	buffer_len_++;

	if (buffer_len_ == buffer_.size())
		send();

	return 1;
}

size_t WebServer::Request::write(const uint8_t *buffer, size_t size) {
	size_t written = 0;

	while (size > 0) {
		auto remaining = std::min(size, buffer_.size() - buffer_len_);

		std::memcpy(&buffer_[buffer_len_], buffer + written, remaining);
		buffer_len_ += remaining;

		size -= remaining;
		written += remaining;

		if (buffer_len_ == buffer_.size())
			send();
	}

	return written;
}

void WebServer::Request::send() {
	if (buffer_len_ > 0) {
		if (send_err_ == ESP_OK)
			send_err_ = httpd_resp_send_chunk(req_, buffer_.data(), buffer_len_);
		buffer_len_ = 0;
		sent_ = true;
	}
}

void WebServer::Request::finish() {
	if (sent_) {
		send();
		httpd_resp_send_chunk(req_, nullptr, 0);
	} else {
		if (!status_)
			httpd_resp_set_status(req_, HTTPD_204);

		httpd_resp_send(req_, buffer_.data(), buffer_len_);
	}
}
#endif

const std::string_view WebServer::Request::uri() const {
#ifdef ENV_NATIVE
	return url_;
#else
	return req_->uri;
#endif
}

void WebServer::Request::set_status(unsigned int status) {
#ifdef ENV_NATIVE
	status_ = status;
#else
	if (status == 200) {
		httpd_resp_set_status(req_, HTTPD_200);
	} else if (status == 303) {
		httpd_resp_set_status(req_, "303 See Other");
	} else if (status == 400) {
		httpd_resp_set_status(req_, HTTPD_400);
	} else if (status == 404) {
		httpd_resp_set_status(req_, HTTPD_404);
	} else if (status == 413) {
		httpd_resp_set_status(req_, "413 Request Entity Too Large");
	} else {
		httpd_resp_set_status(req_, HTTPD_500);
	}
	status_ = true;
#endif
}

void WebServer::Request::set_type(const char *type) {
#ifdef ENV_NATIVE
	content_type_ = type;
#else
	httpd_resp_set_type(req_, type);
#endif
}

void WebServer::Request::add_header(const char *name, const char *value) {
#ifdef ENV_NATIVE
	resp_headers_.emplace(name, value);
#else
	httpd_resp_set_hdr(req_, name, value);
#endif
}

void WebServer::Request::add_header(const char *name, const std::string &value) {
#ifdef ENV_NATIVE
	resp_headers_.emplace(name, value);
#else
	resp_headers_.emplace_back(strdup(value.c_str()));
	add_header(name, resp_headers_.back().get());
#endif
}

std::string WebServer::Request::get_header(const char *name) {
#ifdef ENV_NATIVE
	return MHD_lookup_connection_value(connection_, MHD_HEADER_KIND, name);
#else
	size_t len = httpd_req_get_hdr_value_len(req_, name);

	if (len == 0)
		return "";

	std::vector<char> buffer(len + 1);

	if (httpd_req_get_hdr_value_str(req_, name, buffer.data(), buffer.size()) != ESP_OK)
		return "";

	return buffer.data();
#endif
}

#ifdef ENV_NATIVE
void* WebServer::log_connection(void *cls, const char *uri,
		struct MHD_Connection *connection) {
	return new Request{connection, uri};
}

int WebServer::handle_connection(void *cls,
		struct MHD_Connection *connection, const char *url, const char *method,
		const char *version, const char *upload_data, size_t *upload_data_size,
		void **con_cls) {
	WebServer *server = reinterpret_cast<WebServer*>(cls);
	return server->handle_connection(connection, url, method, upload_data,
		upload_data_size, reinterpret_cast<Request**>(con_cls));
}

void WebServer::cleanup_connection(void *cls, struct MHD_Connection *connection,
		void **con_cls, enum MHD_RequestTerminationCode toe) {
	Request *req = reinterpret_cast<Request*>(*con_cls);
	if (req) {
		delete req;
		*con_cls = nullptr;
	}
}

int WebServer::handle_connection(struct MHD_Connection *connection,
		const char *url, const char *method, const char *upload_data,
		size_t *upload_data_size, Request **req) {
	for (auto &uri_handler : uri_handlers_) {
		if (uri_handler->method() != method)
			continue;

		if (uri_handler->uri() != url)
			continue;

		if (*req) {
			if ((**req).first()) {
				return MHD_YES;
			} else if (upload_data && *upload_data_size) {
				(**req).upload(upload_data, *upload_data_size);
				*upload_data_size = 0;
				return MHD_YES;
			} else {
				return uri_handler->handle_connection(**req);
			}
		} else {
			return MHD_NO;
		}
	}

	const char *message = "404 Not Found";
	auto response = wrap_response(MHD_create_response_from_buffer(strlen(message),
			(void*)message, MHD_RESPMEM_PERSISTENT));
	auto ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response.get());

	MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain");
	return ret;
}
#endif

} // namespace aurcor
