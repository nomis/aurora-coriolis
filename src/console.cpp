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
#include "aurcor/led_profile.h"
#include "aurcor/preset.h"
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

namespace aurcor {

static constexpr inline AppShell &to_app_shell(Shell &shell) {
	return static_cast<AppShell&>(shell);
}

static constexpr inline App &to_app(Shell &shell) {
	return static_cast<App&>(to_app_shell(shell).app_);
}

static constexpr inline AurcorShell &to_shell(Shell &shell) {
	return static_cast<AurcorShell&>(shell);
}

namespace console {

__attribute__((noinline))
static void led_profile_result(AurcorShell &shell, LEDProfile::Result result = LEDProfile::Result::OK, const __FlashStringHelper *message = nullptr) {
	switch (result) {
	case LEDProfile::Result::OK:
		if (message) {
			shell.println(message);
		} else {
			shell.profile().print(shell, shell.bus()->length());
		}
		break;

	case LEDProfile::Result::FULL:
		shell.println(F("Profile full"));
		break;

	case LEDProfile::Result::OUT_OF_RANGE:
		shell.println(F("Index out of range"));
		break;

	case LEDProfile::Result::NOT_FOUND:
		shell.println(F("Index not found"));
		break;

	case LEDProfile::Result::PARSE_ERROR:
		shell.println(F("File parse error"));
		break;

	case LEDProfile::Result::IO_ERROR:
		shell.println(F("File I/O error"));
		break;
	}
}

__attribute__((noinline))
static std::vector<std::string> bus_names_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	return to_app(shell).bus_names();
}

__attribute__((noinline))
static std::vector<std::string> profile_names_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto profiles = LEDProfiles::lc_names();

	std::sort(profiles.begin(), profiles.end());
	return profiles;
};

__attribute__((noinline))
static std::vector<std::string> bus_profile_names_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		return bus_names_autocomplete(shell, current_arguments, next_argument);
	} else if (current_arguments.size() == 1) {
		return profile_names_autocomplete(shell, current_arguments, next_argument);
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> indexes_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	std::vector<std::string> indexes;

	for (auto &value : to_shell(shell).profile().indexes()) {
		indexes.emplace_back(std::to_string(value));
	}

	return indexes;
}

__attribute__((noinline))
static std::vector<std::string> script_names_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto names = MicroPythonFile::scripts();

	std::sort(names.begin(), names.end());
	return names;
}

__attribute__((noinline))
static std::vector<std::string> bus_script_names_autocomplete(Shell &shell,
			const std::vector<std::string> &current_arguments,
			const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		return bus_names_autocomplete(shell, current_arguments, next_argument);
	} else if (current_arguments.size() == 1) {
		return script_names_autocomplete(shell, current_arguments, next_argument);
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> bus_profile_index_values_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.empty()) {
		return indexes_autocomplete(shell, current_arguments, next_argument);
	} else {
		uint8_t r, g, b;
		auto index = std::atol(current_arguments[0].c_str());

		if (to_shell(shell).profile().get(index, r, g, b) == LEDProfile::OK) {
			switch (current_arguments.size()) {
			case 1:
				return {std::to_string(r)};

			case 2:
				return {std::to_string(g)};

			case 3:
				return {std::to_string(b)};
			}
		}

		return {};
	}
}

namespace main {

/* <bus> */
static void bus(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus_name = arguments[0];
	auto bus = to_app(shell).bus(bus_name);

	if (bus) {
		to_shell(shell).enter_bus_context(bus);
	} else {
		shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
	}
}

/* <bus> <profile> */
static void profile(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus_name = arguments[0];
	auto &profile_name = arguments[1];
	auto &app = to_app(shell);
	auto bus = app.bus(bus_name);

	if (bus) {
		enum led_profile_id profile_id;

		if (LEDProfiles::lc_id(profile_name, profile_id)) {
			auto &aurcor_shell = to_shell(shell);

			aurcor_shell.enter_bus_profile_context(bus, profile_id);
			led_profile_result(aurcor_shell);
		} else {
			shell.printfln(F("Profile \"%s\" not found"), profile_name.c_str());
		}
	} else {
		shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
	}
}

/* <bus> */
static void mpy(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus_name = arguments[0];
	auto &app = to_app(shell);
	auto bus = app.bus(bus_name);
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

				app.stop(bus);

				if (app.detach(bus)) {
					mp = std::make_shared<MicroPythonShell>(
						to_app_shell(shell).console_name(), bus,
						std::make_shared<Preset>(app, bus));

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
}

/* <bus> <script> */
static void run(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus_name = arguments[0];
	auto &script_name = arguments[1];
	auto &app = to_app(shell);
	auto bus = app.bus(bus_name);

	if (bus) {
		if (MicroPythonFile::exists(script_name.c_str())) {
			auto preset = std::make_shared<Preset>(app, bus);
			preset->script(script_name);
			app.start(bus, preset);
		} else {
			shell.printfln(F("Script \"%s\" not found"), script_name.c_str());
		}
	} else {
		shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
	}
}

/* <bus> */
static void stop(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus_name = arguments[0];
	auto &app = to_app(shell);
	auto bus = app.bus(bus_name);

	if (bus) {
		app.stop(bus);
		app.detach(bus);
	} else {
		shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
	}
}

} // namespace main

namespace bus {

__attribute__((noinline))
static void show_length(Shell &shell, const std::vector<std::string> &arguments) {
	shell.printfln(F("Length: %zu"), to_shell(shell).bus()->length());
};

__attribute__((noinline))
static void show_direction(Shell &shell, const std::vector<std::string> &arguments) {
	shell.printfln(F("Direction: %s"), to_shell(shell).bus()->reverse() ? "reverse" : "normal");
};

static void edit(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);
	auto preset = to_app(shell).edit(aurcor_shell.bus());

	if (preset) {
		aurcor_shell.enter_bus_preset_context(preset);
	} else {
		shell.printfln(F("Preset not running"));
	}
}

/* [length] */
static void length(Shell &shell, const std::vector<std::string> &arguments) {
	if (!arguments.empty() && shell.has_any_flags(CommandFlags::ADMIN)) {
		to_shell(shell).bus()->length(std::atol(arguments[0].c_str()));
	}
	show_length(shell, {});
}

static void normal(Shell &shell, const std::vector<std::string> &arguments) {
	to_shell(shell).bus()->reverse(false);
	show_direction(shell, {});
}

/* <profile> */
static void profile(Shell &shell, const std::vector<std::string> &arguments) {
	auto &profile_name = arguments[0];
	enum led_profile_id profile_id;

	if (LEDProfiles::lc_id(profile_name, profile_id)) {
		auto &aurcor_shell = to_shell(shell);

		aurcor_shell.enter_bus_profile_context(profile_id);
		led_profile_result(aurcor_shell);
	} else {
		shell.printfln(F("Profile \"%s\" not found"), profile_name.c_str());
	}
}

static void reverse(Shell &shell, const std::vector<std::string> &arguments) {
	to_shell(shell).bus()->reverse(true);
	show_direction(shell, {});
}

/* <script> */
static void run(Shell &shell, const std::vector<std::string> &arguments) {
	auto &script_name = arguments[0];
	auto &app = to_app(shell);
	auto &bus = to_shell(shell).bus();

	if (MicroPythonFile::exists(script_name.c_str())) {
		auto preset = std::make_shared<Preset>(app, bus);
		preset->script(script_name);
		app.start(bus, preset);
	} else {
		shell.printfln(F("Script \"%s\" not found"), script_name.c_str());
	}
}

static void stop(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);
	auto &bus = to_shell(shell).bus();

	app.stop(bus);
	app.detach(bus);
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	show_length(shell, {});
	show_direction(shell, {});
}

} // namespace bus

namespace bus_profile {

/* <index> <+/- red> <+/- green> <+/- blue> */
static void adjust(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.profile().adjust(std::atol(arguments[0].c_str()),
		std::atol(arguments[1].c_str()), std::atol(arguments[2].c_str()), std::atol(arguments[3].c_str())));
}

static void compact(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	aurcor_shell.profile().compact();
	led_profile_result(aurcor_shell);
}

/* <index> <index> */
static void cp(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.profile().copy(std::atol(arguments[0].c_str()),
		std::atol(arguments[1].c_str())));
}

/* <index> <index> */
static void mv(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.profile().move(std::atol(arguments[0].c_str()),
		std::atol(arguments[1].c_str())));
}

static void reload(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.bus()->load_profile(aurcor_shell.profile_id()));
}

static void reset(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	aurcor_shell.profile().clear();
	led_profile_result(aurcor_shell);
}

/* <index> */
static void rm(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.profile().remove(std::atol(arguments[0].c_str())));
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	led_profile_result(to_shell(shell));
}

/* <index> <red> <green> <blue> */
static void set(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.profile().set(std::atol(arguments[0].c_str()),
		std::atol(arguments[1].c_str()), std::atol(arguments[2].c_str()), std::atol(arguments[3].c_str())));
}

static void save(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	led_profile_result(aurcor_shell, aurcor_shell.bus()->save_profile(aurcor_shell.profile_id()), F("Saved"));
}

} // namespace profile

namespace context {

static constexpr auto main = ShellContext::MAIN;
static constexpr auto bus = ShellContext::BUS;
static constexpr auto bus_profile = ShellContext::BUS_PROFILE;
static constexpr auto bus_preset = ShellContext::BUS_PRESET;

__attribute__((noinline))
static void exit(Shell &shell, const std::vector<std::string> &arguments) {
	shell.exit_context();
}

} // namespace context

static constexpr auto user = CommandFlags::USER;
static constexpr auto admin = CommandFlags::ADMIN;

} // namespace console

using namespace console;

static inline void setup_commands(std::shared_ptr<Commands> &commands) {
	commands->add_command(context::main, user, {F("bus")}, {F("<bus>")}, main::bus, bus_names_autocomplete);
	commands->add_command(context::main, user, {F("mpy")}, {F("<bus>")}, main::mpy, bus_names_autocomplete);
	commands->add_command(context::main, user, {F("profile")}, {F("<bus>"), F("<profile>")}, main::profile, bus_profile_names_autocomplete);
	commands->add_command(context::main, user, {F("run")}, {F("<bus>"), F("<script>")}, main::run, bus_script_names_autocomplete);
	commands->add_command(context::main, user, {F("stop")}, {F("<bus>")}, main::stop, bus_names_autocomplete);

	commands->add_command(context::bus, admin, {F("edit")}, bus::edit);
	commands->add_command(context::bus, user, {F("exit")}, context::exit);
	commands->add_command(context::bus, user, {F("help")}, AppShell::main_help_function);
	commands->add_command(context::bus, user, {F("length")}, {F("[length]")}, bus::length);
	commands->add_command(context::bus, user, {F("logout")}, AppShell::main_logout_function);
	commands->add_command(context::bus, admin, {F("normal")}, bus::normal);
	commands->add_command(context::bus, user, {F("profile")}, {F("<profile>")}, bus::profile, profile_names_autocomplete);
	commands->add_command(context::bus, admin, {F("reverse")}, bus::reverse);
	commands->add_command(context::bus, user, {F("run")}, {F("<script>")}, bus::run, script_names_autocomplete);
	commands->add_command(context::bus, user, {F("stop")}, bus::stop);
	commands->add_command(context::bus, user, {F("show")}, bus::show);

	commands->add_command(context::bus_profile, admin, {F("adjust")},
			{F("<index>"), F("<+/- red>"), F("<+/- green>"), F("<+/- blue>")},
			bus_profile::adjust, indexes_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("compact")}, bus_profile::compact);
	commands->add_command(context::bus_profile, admin, {F("cp")}, {F("<index>"), F("<index>")}, bus_profile::cp, indexes_autocomplete);
	commands->add_command(context::bus_profile, user, {F("exit")}, context::exit);
	commands->add_command(context::bus_profile, user, {F("help")}, AppShell::main_help_function);
	commands->add_command(context::bus_profile, user, {F("logout")}, AppShell::main_logout_function);
	commands->add_command(context::bus_profile, admin, {F("mv")}, {F("<index>"), F("<index>")}, bus_profile::mv, indexes_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("reload")}, bus_profile::reload);
	commands->add_command(context::bus_profile, admin, {F("reset")}, bus_profile::reset);
	commands->add_command(context::bus_profile, admin, {F("rm")}, {F("<index>")}, bus_profile::rm, indexes_autocomplete);
	commands->add_command(context::bus_profile, user, {F("show")}, bus_profile::show);
	commands->add_command(context::bus_profile, admin, {F("set")},
			{F("<index>"), F("<red>"), F("<green>"), F("<blue>")},
			bus_profile::set, bus_profile_index_values_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("save")}, bus_profile::save);

	commands->add_command(context::bus_preset, user, {F("exit")}, context::exit);
	commands->add_command(context::bus_preset, user, {F("help")}, AppShell::main_help_function);
	commands->add_command(context::bus_preset, user, {F("logout")}, AppShell::main_logout_function);
}

AurcorShell::AurcorShell(app::App &app, Stream &stream, unsigned int context, unsigned int flags)
	: AppShell(app, stream, context, flags) {

}

void AurcorShell::enter_bus_context(std::shared_ptr<LEDBus> bus) {
	if (context() == ShellContext::MAIN) {
		enter_context(ShellContext::BUS);
		bus_ = bus;
	}
}

void AurcorShell::enter_bus_profile_context(enum led_profile_id profile) {
	if (context() == ShellContext::BUS) {
		enter_context(ShellContext::BUS_PROFILE);
		profile_ = profile;
	}
}

void AurcorShell::enter_bus_profile_context(std::shared_ptr<LEDBus> bus, enum led_profile_id profile) {
	if (context() == ShellContext::MAIN) {
		enter_context(ShellContext::BUS_PROFILE);
		bus_ = bus;
		profile_ = profile;
	}
}

void AurcorShell::enter_bus_preset_context(std::shared_ptr<std::shared_ptr<Preset>> preset) {
	if (context() == ShellContext::BUS) {
		enter_context(ShellContext::BUS_PRESET);
		preset_ = preset;
	}
}

bool AurcorShell::exit_context() {
	auto prev_context = context();
	bool ret = AppShell::exit_context();
	auto new_context = context();
	if (prev_context == ShellContext::BUS || new_context == ShellContext::MAIN) {
		bus_.reset();
	}
	if (prev_context == ShellContext::BUS_PRESET || new_context == ShellContext::MAIN) {
		preset_.reset();
	}
	return ret;
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

	case ShellContext::BUS_PROFILE: {
			std::string text{"/bus/"};

			text.append(bus_->name());
			text.append("/profile/");
			text.append(LEDProfiles::lc_name(profile_));

			if (profile().modified())
				text.append("(unsaved)");

			return text;
		}

	case ShellContext::BUS_PRESET: {
			std::string text{"/bus/"};

			text.append(bus_->name());
			text.append("/preset/");
			text.append(preset_->get()->name().c_str());

			if (preset_->get()->modified())
				text.append("(unsaved)");

			return text;
		}

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
