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

#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <uuid/console.h>
#include <uuid/log.h>

#include "aurcor/app.h"
#include "aurcor/micropython.h"
#include "app/config.h"
#include "app/console.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

using ::uuid::flash_string_vector;
using ::uuid::console::Commands;
using ::uuid::console::Shell;
using LogLevel = ::uuid::log::Level;
using LogFacility = ::uuid::log::Facility;

using ::app::CommandFlags;
using ::app::Config;
using ::app::ShellContext;
using ::aurcor::MicroPythonShell;

#define MAKE_PSTR(string_name, string_literal) static const char __pstr__##string_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = string_literal;
#define MAKE_PSTR_WORD(string_name) MAKE_PSTR(string_name, #string_name)
#define F_(string_name) FPSTR(__pstr__##string_name)

namespace aurcor {

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-const-variable"
MAKE_PSTR_WORD(mpy)
MAKE_PSTR(bus_mandatory, "<bus>")
//MAKE_PSTR(xxx, "[xxx]")
#pragma GCC diagnostic pop

static inline app::AppShell &to_app_shell(Shell &shell) {
	return dynamic_cast<app::AppShell&>(shell);
}

static inline App &to_app(Shell &shell) {
	return static_cast<App&>(to_app_shell(shell).app_);
}

static inline void setup_commands(std::shared_ptr<Commands> &commands) {
	#define NO_ARGUMENTS std::vector<std::string>{}

	commands->add_command(ShellContext::MAIN, CommandFlags::USER,
		flash_string_vector{F_(mpy)}, flash_string_vector{F_(bus_mandatory)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		auto &bus_name = arguments[0];
		auto bus = to_app(shell).bus(bus_name);
		std::shared_ptr<MicroPythonShell> mp;

		if (bus) {
			shell.block_with([bus, mp] (Shell &shell, bool stop) mutable -> bool {
				if (mp) {
					if (mp->shell_foreground(shell, stop)) {
						to_app(shell).detach(bus, mp);
						return true;
					}
				} else if (stop) {
					return true;
				} else {
					auto &app = to_app(shell);

					if (app.detach(bus)) {
						mp = std::make_shared<MicroPythonShell>(to_app_shell(shell).console_name(), bus);

						app.attach(bus, mp);
						if (!mp->start(shell)) {
							app.detach(bus, mp);
							return true;
						}
					}
				}

				return false;
			});
		} else {
			shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
		}
	},
	[] (Shell &shell, const std::vector<std::string> &current_arguments __attribute__((unused)),
			const std::string &next_argument) -> std::vector<std::string> {
		return to_app(shell).bus_names();
	});
}

} // namespace aurcor

namespace app {

void setup_commands(std::shared_ptr<Commands> &commands) {
	aurcor::setup_commands(commands);
}

} // namespace app
