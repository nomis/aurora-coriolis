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

#include "aurcor/web_interface.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "app/config.h"
#include "aurcor/app.h"
#include "aurcor/led_bus.h"
#include "aurcor/preset.h"
#include "aurcor/web_server.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "web-interface";

namespace aurcor {

uuid::log::Logger WebInterface::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

static const char * const immutable_headers[][2] = {
	{ "Cache-Control", "public, immutable, max-age=31536000" },
	{ nullptr, nullptr }
};

WebInterface::WebInterface(App &app) : app_(app) {
	using namespace std::placeholders;

	server_.add_get_handler("/", std::bind(&WebInterface::list_presets, this, _1));
	server_.add_post_handler("/preset", std::bind(&WebInterface::set_preset, this, _1));
	server_.add_static_content("/" + app_.immutable_id() + "/list_presets.xml",
		"application/xslt+xml", immutable_headers, nullptr, 0);
}

bool WebInterface::list_presets(WebServer::Request &req) {
	req.set_status(200);
	req.set_type("application/xml");
	req.add_header("Cache-Control", "no-cache");

	app::Config config;
	std::string default_preset;
	std::string current_preset;

	if (!config.default_bus().empty()) {
		auto bus = app_.bus(config.default_bus());

		if (bus) {
			default_preset = bus->default_preset();
			current_preset = app_.current_preset_name(bus);
		}
	}

	req.printf(
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<?xml-stylesheet type=\"text/xsl\" href=\"/%s/list_presets.xml\"?>"
			"<l>", app_.immutable_id().c_str()
	);

	auto presets = app_.preset_descriptions();

	for (auto &preset : presets.first) {
		bool is_current = preset.first == current_preset;
		bool is_default = preset.first == default_preset;

		req.printf("<p n=\"%s\" d=\"%s\"", preset.first.c_str(), preset.second.c_str());
		if (is_current || is_default) {
			req.printf(" f=\"%s%s\"", is_current ? "r" : "", is_default ? "d" : "");
		}
		req.print("/>");
	}

	req.print("</l>");
	return true;
}

bool WebInterface::set_preset(WebServer::Request &req) {
	if (req.get_header("Content-Type") != "application/x-www-form-urlencoded") {
		req.set_status(400);
		return true;
	}

	size_t len = req.available();

	if (len > 256) {
		req.set_status(413);
		return true;
	}

	std::vector<char> buffer(len);
	bool success = false;

	req.readBytes(buffer.data(), buffer.size());

	const std::string_view name_prefix{"name="};
	std::string_view text{buffer.data(), buffer.size()};
	auto params = parse_form(text);
	std::string_view preset_name;
	const char *error = "";

	auto it = params.find("name");
	if (it != params.end())
		preset_name = it->second;

	if (preset_name.empty()) {
		error = "No preset specified";
	} else {
		app::Config config;

		if (config.default_bus().empty()) {
			error = "No default bus";
		} else {
			auto bus = app_.bus(config.default_bus());
			auto preset = std::make_shared<Preset>(app_, bus);

			if (!preset->name(preset_name)) {
				error = "Invalid preset name";
			} else {
				auto result = preset->load();

				switch (result) {
				case Result::OK:
					if (!app_.start(bus, preset, false)) {
						error = "Access denied: current preset is unsaved";
					} else {
						success = true;
					}
					break;

				case Result::NOT_FOUND:
					error = "Preset not found";
					break;

				case Result::FULL:
				case Result::OUT_OF_RANGE:
					error = "Preset too large or invalid";
					break;

				case Result::PARSE_ERROR:
				case Result::IO_ERROR:
				default:
					error = "Failed to load preset";
					break;
				}
			}
		}
	}

	req.add_header("Cache-Control", "no-cache");
	if (success) {
		req.set_status(303);
		req.set_type("text/plain");
		req.add_header("Location", "/");
	} else {
		req.set_status(200);
		req.set_type("text/html");
		req.printf(
			"<!DOCTYPE html><html><head>"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			"<meta http-equiv=\"refresh\" content=\"2;URL=/\">"
			"</head><body><p>%s</p></body></html>", error);
	}
	return true;
}

std::unordered_map<std::string_view,std::string_view>
		WebInterface::parse_form(std::string_view text) {
	std::unordered_map<std::string_view,std::string_view> params;

	while (text.length() > 0) {
		std::string_view value;
		auto amp_pos = text.find('&');

		if (amp_pos != std::string_view::npos) {
			value = text.substr(0, amp_pos);
			text.remove_prefix(amp_pos + 1);
		} else {
			value = text;
			text = {};
		}

		auto eq_pos = value.find('=');

		if (eq_pos != std::string_view::npos) {
			params.emplace(value.substr(0, eq_pos), value.substr(eq_pos + 1));
		} else {
			params.emplace(value, "");
		}
	}

	return params;
}

} // namespace aurcor
