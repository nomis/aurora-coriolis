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

#pragma once

#include <Arduino.h>

#ifdef ENV_NATIVE
# include <sys/types.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <microhttpd.h>
# if MHD_VERSION < 0x00097002
typedef int MHD_Result;
# endif
#else
# include <esp_http_server.h>
#endif

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <uuid/log.h>

namespace aurcor {

class WebServer {
public:
#ifdef ENV_NATIVE
	static constexpr uint16_t DEFAULT_PORT = 0;
#else
	static constexpr uint16_t DEFAULT_PORT = 80;
#endif

	class Request: public Stream {
		friend WebServer;
	public:
#ifdef ENV_NATIVE
		Request(struct MHD_Connection *connection, const char *url);
#else
		Request(httpd_req_t *req);
#endif

		int available() override;
		int read() override;
		int peek() override;
		size_t readBytes(char *buffer, size_t length) override;

		size_t write(uint8_t c) override;
		size_t write(const uint8_t *buffer, size_t size) override;

		const std::string_view uri() const;
		std::string get_header(const char *name);

		void set_status(unsigned int status);
		void set_type(const char *type);
		void add_header(const char *name, const char *value);
		void add_header(const char *name, const std::string &value);

	private:
#ifdef ENV_NATIVE
		bool first();
		void upload(const char *data, size_t len);
		MHD_Result finish();

		struct MHD_Connection *connection_;
		const std::string url_;
		bool first_{true};
		std::vector<char> upload_data_;
		std::vector<char> buffer_;
		unsigned int status_{0};
		const char *content_type_;
		std::unordered_map<std::string,std::string> resp_headers_;
#else
		void send();
		void finish();

		httpd_req_t *req_;
		size_t content_len_;
		esp_err_t send_err_{ESP_OK};
		std::vector<char> buffer_;
		size_t buffer_len_{0};
		std::vector<std::unique_ptr<char>> resp_headers_;
		bool status_{false};
		bool sent_{false};
#endif
	};

	using get_function = std::function<bool(Request &req)>;
	using post_function = std::function<bool(Request &req)>;

	WebServer(uint16_t port = DEFAULT_PORT);
	~WebServer();

	bool add_get_handler(const std::string &uri, get_function handler);
	bool add_post_handler(const std::string &uri, post_function handler);
	bool add_static_content(const std::string &uri, const char *content_type,
		const char * const headers[][2], const std::string_view data);

private:
#ifdef ENV_NATIVE
	class MHD_DaemonDeleter {
	public:
		void operator()(struct MHD_Daemon *daemon) {
			MHD_stop_daemon(daemon);
		}
	};

	class MHD_ResponseDeleter {
	public:
		void operator()(struct MHD_Response *response) {
			MHD_destroy_response(response);
		}
	};
#else
	class HandleDeleter {
	public:
		void operator()(httpd_handle_t handle) {
			httpd_stop(handle);
		}
	};
#endif

	class URIHandler {
	public:
		virtual ~URIHandler() = default;

#ifdef ENV_NATIVE
		virtual std::string method() = 0;
		inline const std::string& uri() const { return uri_; }
		virtual MHD_Result handle_connection(Request &req) = 0;
#else
		virtual httpd_method_t method() = 0;
		bool server_register(httpd_handle_t server);
		void server_unregister(httpd_handle_t server);
#endif

	protected:
		URIHandler(const std::string &uri);

#ifndef ENV_NATIVE
		virtual esp_err_t handler_function(httpd_req_t *req) = 0;
#endif

	private:
		const std::string uri_;
	};

	class GetURIHandler: public URIHandler {
	public:
		GetURIHandler(const std::string &uri, get_function handler);

#ifdef ENV_NATIVE
		std::string method() override;
		MHD_Result handle_connection(Request &req) override;
#endif

	protected:
#ifndef ENV_NATIVE
		httpd_method_t method() override;
		esp_err_t handler_function(httpd_req_t *req) override;
#endif

	private:
		get_function function_;
	};

	class PostURIHandler: public URIHandler {
	public:
		PostURIHandler(const std::string &uri, post_function handler);

#ifdef ENV_NATIVE
		std::string method() override;
		MHD_Result handle_connection(Request &req) override;
#endif

	protected:
#ifndef ENV_NATIVE
		httpd_method_t method() override;
		esp_err_t handler_function(httpd_req_t *req) override;
#endif

	private:
		post_function function_;
	};

	class StaticContentURIHandler: public URIHandler {
	public:
		StaticContentURIHandler(const std::string &uri, const char *content_type,
			const char * const headers[][2], const std::string_view data);

#ifdef ENV_NATIVE
		std::string method() override;
		MHD_Result handle_connection(Request &req) override;
#endif

	protected:
#ifndef ENV_NATIVE
		httpd_method_t method() override;
		esp_err_t handler_function(httpd_req_t *req) override;
#endif

	private:
		const char *content_type_;
		const char * const (*headers_)[2];
		const std::string_view data_;
	};

#ifdef ENV_NATIVE
	static inline std::unique_ptr<struct MHD_Response,MHD_ResponseDeleter>
			wrap_response(struct MHD_Response *response) {
		return std::unique_ptr<struct MHD_Response,MHD_ResponseDeleter>(response);
	}

	static void* log_connection(void *cls, const char *uri,
		struct MHD_Connection *connection);

	static MHD_Result handle_connection(void *cls,
		struct MHD_Connection *connection, const char *url, const char *method,
		const char *version, const char *upload_data, size_t *upload_data_size,
		void **con_cls);

	static void cleanup_connection(void *cls, struct MHD_Connection *connection,
		void **con_cls, enum MHD_RequestTerminationCode toe);
#endif

	static uuid::log::Logger logger_;

#ifdef ENV_NATIVE
	MHD_Result handle_connection(struct MHD_Connection *connection, const char *url,
		const char *method, const char *upload_data, size_t *upload_data_size,
		Request **req);

	std::unique_ptr<struct MHD_Daemon,MHD_DaemonDeleter> daemon_;
#else
	std::unique_ptr<void,HandleDeleter> handle_;
#endif
	std::vector<std::unique_ptr<URIHandler>> uri_handlers_;
};

} // namespace aurcor
