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

#include <uuid/log.h>

#include "app.h"
#include "web_server.h"

namespace aurcor {

class WebServer;

class WebInterface {
public:
	WebInterface(App &app);

private:
	static std::unordered_map<std::string_view,std::string_view> parse_form(std::string_view text);

	bool list_presets(WebServer::Request &req);
	bool set_preset(WebServer::Request &req);

	static uuid::log::Logger logger_;

	App &app_;
	WebServer server_;
};

} // namespace aurcor
