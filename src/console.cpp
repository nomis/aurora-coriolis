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
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <uuid/console.h>
#include <uuid/log.h>

#include "aurcor/app.h"
#include "aurcor/console.h"
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

using ::app::AppShell;
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
MAKE_PSTR_WORD(bus)
MAKE_PSTR_WORD(exit)
MAKE_PSTR_WORD(help)
MAKE_PSTR_WORD(length)
MAKE_PSTR_WORD(logout)
MAKE_PSTR_WORD(mpy)
MAKE_PSTR_WORD(normal)
MAKE_PSTR_WORD(reverse)
MAKE_PSTR_WORD(show)
MAKE_PSTR(bus_mandatory, "<bus>")
MAKE_PSTR(length_optional, "[length]")
//MAKE_PSTR(xxx, "[xxx]")
#pragma GCC diagnostic pop

static inline AppShell &to_app_shell(Shell &shell) {
	return dynamic_cast<AppShell&>(shell);
}

static inline App &to_app(Shell &shell) {
	return static_cast<App&>(to_app_shell(shell).app_);
}

static inline AurcorShell &to_shell(Shell &shell) {
	return dynamic_cast<AurcorShell&>(shell);
}

static inline void setup_commands(std::shared_ptr<Commands> &commands) {
	#define NO_ARGUMENTS std::vector<std::string>{}

	auto bus_names_autocomplete = [] (Shell &shell,
			const std::vector<std::string> &current_arguments,
			const std::string &next_argument) -> std::vector<std::string> {
		return to_app(shell).bus_names();
	};

	commands->add_command(ShellContext::MAIN, CommandFlags::USER,
		flash_string_vector{F_(bus)}, flash_string_vector{F_(bus_mandatory)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		auto &bus_name = arguments[0];
		auto bus = to_app(shell).bus(bus_name);

		if (bus) {
			to_shell(shell).enter_bus_context(bus);
		} else {
			shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
		}
	}, bus_names_autocomplete);

	auto bus_exit_function = [] (Shell &shell, const std::vector<std::string> &arguments) {
		shell.exit_context();
	};

	commands->add_command(ShellContext::BUS, CommandFlags::USER, flash_string_vector{F_(exit)}, bus_exit_function);

	commands->add_command(ShellContext::BUS, CommandFlags::USER, flash_string_vector{F_(help)},
			[] (Shell &shell, const std::vector<std::string> &arguments) {
		shell.print_all_available_commands();
	});

	auto show_length = [] (Shell &shell, const std::vector<std::string> &arguments) {
		shell.printfln(F("Length: %zu"), to_shell(shell).bus()->length());
	};

	auto show_direction = [] (Shell &shell, const std::vector<std::string> &arguments) {
		shell.printfln(F("Direction: %s"), to_shell(shell).bus()->reverse() ? "reverse" : "normal");
	};

	commands->add_command(ShellContext::BUS, CommandFlags::USER, flash_string_vector{F_(length)}, flash_string_vector{F_(length_optional)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		if (!arguments.empty() && shell.has_any_flags(CommandFlags::ADMIN)) {
			to_shell(shell).bus()->length(std::atol(arguments[0].c_str()));
		}
		show_length(shell, NO_ARGUMENTS);
	});

	commands->add_command(ShellContext::BUS, CommandFlags::USER, flash_string_vector{F_(logout)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		bus_exit_function(shell, NO_ARGUMENTS);
		AppShell::main_logout_function(shell, NO_ARGUMENTS);
	});

	commands->add_command(ShellContext::BUS, CommandFlags::ADMIN, flash_string_vector{F_(normal)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		to_shell(shell).bus()->reverse(false);
		show_direction(shell, NO_ARGUMENTS);
	});

	commands->add_command(ShellContext::BUS, CommandFlags::ADMIN, flash_string_vector{F_(reverse)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		to_shell(shell).bus()->reverse(true);
		show_direction(shell, NO_ARGUMENTS);
	});

	commands->add_command(ShellContext::BUS, CommandFlags::ADMIN, flash_string_vector{F_(show)},
			[=] (Shell &shell, const std::vector<std::string> &arguments) {
		show_length(shell, NO_ARGUMENTS);
		show_direction(shell, NO_ARGUMENTS);
	});

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
	}, bus_names_autocomplete);
}

AurcorShell::AurcorShell(app::App &app) : Shell(), AppShell(app) {

}

void AurcorShell::enter_bus_context(std::shared_ptr<LEDBus> bus) {
	if (context() == ShellContext::MAIN) {
		enter_context(ShellContext::BUS);
		bus_ = bus;
	}
}

bool AurcorShell::exit_context() {
	if (context() == ShellContext::BUS) {
		bus_.reset();
	}
	return AppShell::exit_context();
}

void AurcorShell::display_banner() {
	AppShell::display_banner();
	println(F("┌─────────────────────────────────────────────────────────────────┐"));
	println(F("│Curtains of octarine glow danced slowly and majestically over the│"));
	println(F("│Disc as the fire of the  Aurora Coriolis,  the vast discharge of │"));
	println(F("│magic from the Disc’s standing field, earthed itself in the green│"));
	println(F("│ice mountains of the Hub. The central spire of Cori Celesti, home│"));
	println(F("│of the gods, was a ten mile high column of cold coruscating fire.│"));
	println(F("└─────────────────────────────────────────────────────────────────┘"));
	println();
}

std::string AurcorShell::context_text() {
	switch (static_cast<ShellContext>(context())) {
	case ShellContext::BUS:
		return std::string{"/bus/"} + bus_->name();

	default:
		break;
	}

	return AppShell::context_text();
}

} // namespace aurcor

namespace app {

void setup_commands(std::shared_ptr<Commands> &commands) {
	aurcor::setup_commands(commands);
}

} // namespace app
