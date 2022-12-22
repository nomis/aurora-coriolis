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

namespace bus_preset {
	static void show(Shell &shell, const std::vector<std::string> &arguments);
}

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
		std::vector<std::string> values;
		auto bus_names = bus_names_autocomplete(shell, current_arguments, next_argument);
		auto profile_names = profile_names_autocomplete(shell, current_arguments, next_argument);

		values.insert(values.end(), bus_names.begin(), bus_names.end());
		values.insert(values.end(), profile_names.begin(), profile_names.end());
		return values;
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
static std::vector<std::string> preset_names_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto profiles = Preset::names();

	std::sort(profiles.begin(), profiles.end());
	return profiles;
}

__attribute__((noinline))
static std::vector<std::string> preset_names_default_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		return preset_names_autocomplete(shell, current_arguments, next_argument);
	} else if (current_arguments.size() == 1) {
		return {"default"};
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> bus_script_names_autocomplete(Shell &shell,
			const std::vector<std::string> &current_arguments,
			const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		std::vector<std::string> values;
		auto bus_names = bus_names_autocomplete(shell, current_arguments, next_argument);
		auto script_names = script_names_autocomplete(shell, current_arguments, next_argument);

		values.insert(values.end(), bus_names.begin(), bus_names.end());
		values.insert(values.end(), script_names.begin(), script_names.end());
		return values;
	} else if (current_arguments.size() == 1) {
		return script_names_autocomplete(shell, current_arguments, next_argument);
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> bus_preset_names_default_autocomplete(Shell &shell,
			const std::vector<std::string> &current_arguments,
			const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		std::vector<std::string> values;
		auto bus_names = bus_names_autocomplete(shell, current_arguments, next_argument);
		auto preset_names = preset_names_autocomplete(shell, current_arguments, next_argument);

		values.insert(values.end(), bus_names.begin(), bus_names.end());
		values.insert(values.end(), preset_names.begin(), preset_names.end());
		return values;
	} else if (current_arguments.size() == 1) {
		return preset_names_autocomplete(shell, current_arguments, next_argument);
	} else if (current_arguments.size() == 2) {
		return {"default"};
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

__attribute__((noinline))
static std::vector<std::string> preset_current_name_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active()) {
		auto name = aurcor_shell.preset().name(true);

		if (!name.empty())
			return {name};
	}

	return {};
}

__attribute__((noinline))
static std::vector<std::string> preset_current_description_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active()) {
		return {aurcor_shell.preset().description()};
	}

	return {};
}

static std::shared_ptr<LEDBus> default_bus(Shell &shell) {
	Config config;

	if (config.default_bus().empty()) {
		shell.printfln(F("Default bus not set"));
		return nullptr;
	} else {
		auto bus = to_app(shell).bus(config.default_bus());

		if (!bus) {
			shell.printfln(F("Default bus \"%s\" not found"), config.default_bus().c_str());
		}

		return bus;
	}
}

static std::shared_ptr<LEDBus> lookup_bus_or_default(Shell &shell, const std::vector<std::string> &arguments, size_t index) {
	if (arguments.size() > index) {
		auto &bus_name = arguments[index];
		auto bus = to_app(shell).bus(bus_name);

		if (!bus) {
			shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
		}

		return bus;
	} else {
		return default_bus(shell);
	}
}

namespace bus {

static void show_default_preset(Shell &shell, std::shared_ptr<LEDBus> &bus);

} // namespace bus

namespace main {

/* [bus] */
static void bus(Shell &shell, const std::vector<std::string> &arguments) {
	auto bus = lookup_bus_or_default(shell, arguments, 0);

	if (bus) {
		to_shell(shell).enter_bus_context(bus);
	}
}

/* [bus] <preset> */
static void default_(Shell &shell, const std::vector<std::string> &arguments) {
	const bool has_bus_name = (arguments.size() >= 2);
	auto &preset_name = has_bus_name ? arguments[1] : arguments[0];
	auto bus = has_bus_name ? lookup_bus_or_default(shell, arguments, 0) : default_bus(shell);

	if (bus) {
		if (!preset_name.empty()) {
			auto &app = to_app(shell);
			auto preset = std::make_shared<Preset>(app, bus);

			if (!preset->name(preset_name)) {
				shell.printfln(F("Invalid name"));
				return;
			}

			if (!preset->load()) {
				shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
				return;
			}
		}

		bus->default_preset(preset_name);
		aurcor::console::bus::show_default_preset(shell, bus);
	}
}

/* [bus] */
static void edit(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);
	auto &aurcor_shell = to_shell(shell);
	auto bus = lookup_bus_or_default(shell, arguments, 0);

	if (bus) {
		auto preset = app.edit(bus);

		if (preset) {
			aurcor_shell.enter_bus_preset_context(bus, preset);
			bus_preset::show(shell, {});
		} else {
			shell.printfln(F("Preset not running"));
		}
	}
}

static void list_buses(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);

	shell.printfln(F("Name         Length Direction  Current Preset                                   Default Preset"));
	shell.printfln(F("------------ ------ ---------  ------------------------------------------------ ------------------------------------------------"));

	for (auto &bus_name : app.bus_names()) {
		auto bus = app.bus(bus_name);
		auto preset = app.edit(bus);

		if (bus) {
			shell.printfln(F("%-12s %6u %-9s %c%-48s %-48s"), bus->name(), bus->length(),
				bus->reverse() ? "reverse" : "normal",
				preset ? (preset->get()->modified() ? '*' : ' ') : ' ',
				preset ? preset->get()->name().c_str() : "<none>",
				bus->default_preset().empty() ? "<unset>" : bus->default_preset().c_str());
		}
	}
}

static void list_presets(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);

	shell.printfln(F("Name                                             Description                                      Direction"));
	shell.printfln(F("------------------------------------------------ ------------------------------------------------ ---------"));

	for (auto &preset_name : Preset::names()) {
		auto preset = std::make_shared<Preset>(app, nullptr);

		if (preset->name(preset_name) && preset->load()) {
			shell.printfln(F("%-48s %-48s"),
				preset->name().c_str(), preset->description().c_str(),
				preset->reverse() ? "reverse" : "normal");
		}
	}
}

/* [bus] <profile> */
static void profile(Shell &shell, const std::vector<std::string> &arguments) {
	const bool has_bus_name = (arguments.size() >= 2);
	auto &profile_name = has_bus_name ? arguments[1] : arguments[0];
	auto bus = has_bus_name ? lookup_bus_or_default(shell, arguments, 0) : default_bus(shell);

	if (bus) {
		enum led_profile_id profile_id;

		if (LEDProfiles::lc_id(profile_name, profile_id)) {
			auto &aurcor_shell = to_shell(shell);

			aurcor_shell.enter_bus_profile_context(bus, profile_id);
			led_profile_result(aurcor_shell);
		} else {
			shell.printfln(F("Profile \"%s\" not found"), profile_name.c_str());
		}
	}
}

/* [bus] */
static void mpy(Shell &shell, const std::vector<std::string> &arguments) {
	auto bus = lookup_bus_or_default(shell, arguments, 0);
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
	}
}

/* <preset> <preset> */
static void mv(Shell &shell, const std::vector<std::string> &arguments) {
	auto &preset_from_name = arguments[0];
	auto &preset_to_name = arguments[1];
	auto &app = to_app(shell);
	auto preset_from = std::make_shared<Preset>(app, nullptr);
	auto preset_to = std::make_shared<Preset>(app, nullptr);

	if (!preset_from->name(preset_from_name)) {
		shell.printfln(F("Invalid source name"));
		return;
	}

	if (!preset_to->name(preset_to_name)) {
		shell.printfln(F("Invalid destination name"));
		return;
	}

	if (preset_from->name() == preset_to->name())
		return;

	if (!preset_from->rename(*preset_to)) {
		shell.printfln(F("Preset \"%s\" not found"), preset_from_name.c_str());
		return;
	}

	app.renamed(preset_from_name, preset_to_name);
}

/* <preset> */
static void rm(Shell &shell, const std::vector<std::string> &arguments) {
	auto &preset_name = arguments[0];
	auto &app = to_app(shell);
	auto preset = std::make_shared<Preset>(app, nullptr);

	if (!preset->name(preset_name)) {
		shell.printfln(F("Invalid name"));
		return;
	}

	if (!preset->remove()) {
		shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
		return;
	}

	app.deleted(preset_name);
}

/* [bus] <script> */
static void run(Shell &shell, const std::vector<std::string> &arguments) {
	const bool has_bus_name = (arguments.size() >= 2);
	auto &script_name = has_bus_name ? arguments[1] : arguments[0];
	auto &app = to_app(shell);
	auto bus = has_bus_name ? lookup_bus_or_default(shell, arguments, 0) : default_bus(shell);

	if (bus) {
		if (MicroPythonFile::exists(script_name.c_str())) {
			auto preset = std::make_shared<Preset>(app, bus);
			preset->script(script_name);
			app.start(bus, preset);
		} else {
			shell.printfln(F("Script \"%s\" not found"), script_name.c_str());
		}
	}
}

/* [bus] */
static void set_default_bus(Shell &shell, const std::vector<std::string> &arguments) {
	Config config;

	if (arguments.empty()) {
		config.default_bus("");
	} else {
		auto &bus_name = arguments[0];
		auto &app = to_app(shell);
		auto bus = app.bus(bus_name);

		if (bus) {
			config.default_bus(bus_name);
		} else {
			shell.printfln(F("Bus \"%s\" not found"), bus_name.c_str());
			return;
		}
	}

	config.commit();
	shell.printfln(F("Default bus = %s"), config.default_bus().empty() ? "<unset>" : config.default_bus().c_str());
}

/* [bus] <preset> [default] */
static void start(Shell &shell, const std::vector<std::string> &arguments) {
	const bool has_bus_name = (arguments.size() >= 2);
	auto &preset_name = has_bus_name ? arguments[1] : arguments[0];
	auto &app = to_app(shell);
	auto bus = has_bus_name ? lookup_bus_or_default(shell, arguments, 0) : default_bus(shell);

	if (bus) {
		auto preset = std::make_shared<Preset>(app, bus);

		if (!preset->name(preset_name)) {
			shell.printfln(F("Invalid name"));
			return;
		}

		if (preset->load()) {
			app.start(bus, preset);
		} else {
			shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
			return;
		}

		if (arguments.size() >= 3 && arguments[2] == "default") {
			if (shell.has_any_flags(CommandFlags::ADMIN)) {
				bus->default_preset(preset_name);
				aurcor::console::bus::show_default_preset(shell, bus);
			}
		}
	}
}

/* [bus] */
static void stop(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);
	auto bus = lookup_bus_or_default(shell, arguments, 0);

	if (bus) {
		app.stop(bus);
		app.detach(bus);
	}
}

} // namespace main

namespace bus {

__attribute__((noinline))
static void show_length(Shell &shell) {
	shell.printfln(F("Length: %zu"), to_shell(shell).bus()->length());
};

__attribute__((noinline))
static void show_direction(Shell &shell) {
	shell.printfln(F("Direction: %s"), to_shell(shell).bus()->reverse() ? "reverse" : "normal");
};

__attribute__((noinline))
static void show_default_preset(Shell &shell, std::shared_ptr<LEDBus> &bus) {
	auto default_preset = bus->default_preset();
	shell.printfln(F("Default preset: %s"), default_preset.empty() ? "<unset>" : default_preset.c_str());
};

/* [preset] */
static void default_(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus = to_shell(shell).bus();

	if (!arguments.empty() && shell.has_any_flags(CommandFlags::ADMIN)) {
		auto &preset_name = arguments[0];

		if (!preset_name.empty()) {
			auto &app = to_app(shell);
			auto preset = std::make_shared<Preset>(app, bus);

			if (!preset->name(preset_name)) {
				shell.printfln(F("Invalid name"));
				return;
			}

			if (!preset->load()) {
				shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
				return;
			}
		}

		bus->default_preset(preset_name);
	}

	show_default_preset(shell, bus);
}

/* [preset] */
static void edit(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!arguments.empty()) {
		auto &preset_name = arguments[0];
		auto &app = to_app(shell);
		auto &bus = to_shell(shell).bus();
		auto preset = std::make_shared<Preset>(app, bus);

		if (!preset->name(preset_name)) {
			shell.printfln(F("Invalid name"));
			return;
		}

		if (preset->load()) {
			app.start(bus, preset);
		} else {
			shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
			return;
		}
	}

	auto preset = to_app(shell).edit(aurcor_shell.bus());

	if (preset) {
		aurcor_shell.enter_bus_preset_context(preset);
		bus_preset::show(shell, {});
	} else {
		shell.printfln(F("Preset not running"));
	}
}

/* [length] */
static void length(Shell &shell, const std::vector<std::string> &arguments) {
	if (!arguments.empty() && shell.has_any_flags(CommandFlags::ADMIN)) {
		to_shell(shell).bus()->length(std::atol(arguments[0].c_str()));
	}
	show_length(shell);
}

static void normal(Shell &shell, const std::vector<std::string> &arguments) {
	to_shell(shell).bus()->reverse(false);
	show_direction(shell);
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
	show_direction(shell);
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

/* <preset> [default] */
static void start(Shell &shell, const std::vector<std::string> &arguments) {
	auto &preset_name = arguments[0];
	auto &app = to_app(shell);
	auto &bus = to_shell(shell).bus();
	auto preset = std::make_shared<Preset>(app, bus);

	if (!preset->name(preset_name)) {
		shell.printfln(F("Invalid name"));
		return;
	}

	if (preset->load()) {
		app.start(bus, preset);
	} else {
		shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
		return;
	}

	if (arguments.size() >= 2 && arguments[1] == "default") {
		if (shell.has_any_flags(CommandFlags::ADMIN)) {
			bus->default_preset(preset_name);
			show_default_preset(shell, bus);
		}
	}
}

static void stop(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);
	auto &bus = to_shell(shell).bus();

	app.stop(bus);
	app.detach(bus);
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	show_length(shell);
	show_direction(shell);
	show_default_preset(shell, to_shell(shell).bus());
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

namespace bus_preset {

/* <description> */
static void desc(Shell &shell, const std::vector<std::string> &arguments) {
	auto &description = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!aurcor_shell.preset().description(description))
		shell.printfln(F("Invalid description"));
}

static void reload(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!aurcor_shell.preset().load())
		shell.printfln(F("Failed to reload preset"));
}

/* <name> */
static void name(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!aurcor_shell.preset().name(name))
		shell.printfln(F("Invalid name"));
}

static void normal(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	aurcor_shell.preset().reverse(false);
}

static void reverse(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	aurcor_shell.preset().reverse(true);
}

/* [name] */
static void save(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!arguments.empty()) {
		auto &name = arguments[0];
		if (!aurcor_shell.preset().name(name)) {
			shell.printfln(F("Invalid name"));
			return;
		}
	}

	if (aurcor_shell.preset().name().empty()) {
		shell.printfln(F("Unable to save preset without a name"));
		return;
	}

	if (!aurcor_shell.preset().save())
		shell.printfln(F("Failed to save preset"));

	to_app(shell).refresh(aurcor_shell.preset().name());
}

/* <script> */
static void script(Shell &shell, const std::vector<std::string> &arguments) {
	auto &script_name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (MicroPythonFile::exists(script_name.c_str())) {
		aurcor_shell.preset().script(script_name);
	} else {
		shell.printfln(F("Script \"%s\" not found"), script_name.c_str());
	}
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &preset = aurcor_shell.preset();

	shell.printfln(F("Name:        %s"), preset.name().c_str());
	shell.printfln(F("Description: %s"), preset.description().c_str());
	shell.printfln(F("Script:      %s"), preset.script().c_str());
	shell.printfln(F("Direction:   %s"), preset.reverse() ? "reverse" : "normal");
}

} // namespace bus_preset

namespace context {

static constexpr auto main = ShellContext::MAIN;
static constexpr auto bus = ShellContext::BUS;
static constexpr auto bus_profile = ShellContext::BUS_PROFILE;
static constexpr auto bus_preset = ShellContext::BUS_PRESET;

} // namespace context

static constexpr auto user = CommandFlags::USER;
static constexpr auto admin = CommandFlags::ADMIN;

} // namespace console

using namespace console;

static inline void setup_commands(std::shared_ptr<Commands> &commands) {
	commands->add_command(context::main, user, {F("bus")}, {F("[bus]")}, main::bus, bus_names_autocomplete);
	commands->add_command(context::main, admin, {F("default")}, {F("[bus]"), F("<preset>")}, main::default_, bus_preset_names_default_autocomplete);
	commands->add_command(context::main, admin, {F("edit")}, {F("[bus]")}, main::edit, bus_names_autocomplete);
	commands->add_command(context::main, user, {F("list"), F("buses")}, main::list_buses);
	commands->add_command(context::main, user, {F("list"), F("presets")}, main::list_presets);
	commands->add_command(context::main, user, {F("mpy")}, {F("[bus]")}, main::mpy, bus_names_autocomplete);
	commands->add_command(context::main, admin, {F("mv")}, {F("<preset>"), F("<preset>")}, main::mv, preset_names_autocomplete);
	commands->add_command(context::main, user, {F("profile")}, {F("[bus]"), F("<profile>")}, main::profile, bus_profile_names_autocomplete);
	commands->add_command(context::main, user, {F("run")}, {F("[bus]"), F("<script>")}, main::run, bus_script_names_autocomplete);
	commands->add_command(context::main, admin, {F("rm")}, {F("<preset>")}, main::rm, preset_names_autocomplete);
	commands->add_command(context::main, admin, {F("set"), F("default"), F("bus")}, {F("[bus]")}, main::set_default_bus, bus_names_autocomplete);
	commands->add_command(context::main, user, {F("start")}, {F("[bus]"), F("<preset>"), F("[default]")}, main::start, bus_preset_names_default_autocomplete);
	commands->add_command(context::main, user, {F("stop")}, {F("[bus]")}, main::stop, bus_names_autocomplete);

	commands->add_command(context::bus, user, {F("default")}, {F("[preset]")}, bus::default_, preset_names_autocomplete);
	commands->add_command(context::bus, admin, {F("edit")}, {F("[preset]")}, bus::edit);
	commands->add_command(context::bus, user, {F("length")}, {F("[length]")}, bus::length);
	commands->add_command(context::bus, admin, {F("normal")}, bus::normal);
	commands->add_command(context::bus, user, {F("profile")}, {F("<profile>")}, bus::profile, profile_names_autocomplete);
	commands->add_command(context::bus, admin, {F("reverse")}, bus::reverse);
	commands->add_command(context::bus, user, {F("run")}, {F("<script>")}, bus::run, script_names_autocomplete);
	commands->add_command(context::bus, user, {F("start")}, {F("<preset>"), F("[default]")}, bus::start, preset_names_default_autocomplete);
	commands->add_command(context::bus, user, {F("stop")}, bus::stop);
	commands->add_command(context::bus, user, {F("show")}, bus::show);

	commands->add_command(context::bus_profile, admin, {F("adjust")},
			{F("<index>"), F("<+/- red>"), F("<+/- green>"), F("<+/- blue>")},
			bus_profile::adjust, indexes_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("compact")}, bus_profile::compact);
	commands->add_command(context::bus_profile, admin, {F("cp")}, {F("<index>"), F("<index>")}, bus_profile::cp, indexes_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("mv")}, {F("<index>"), F("<index>")}, bus_profile::mv, indexes_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("reload")}, bus_profile::reload);
	commands->add_command(context::bus_profile, admin, {F("reset")}, bus_profile::reset);
	commands->add_command(context::bus_profile, admin, {F("rm")}, {F("<index>")}, bus_profile::rm, indexes_autocomplete);
	commands->add_command(context::bus_profile, user, {F("show")}, bus_profile::show);
	commands->add_command(context::bus_profile, admin, {F("set")},
			{F("<index>"), F("<red>"), F("<green>"), F("<blue>")},
			bus_profile::set, bus_profile_index_values_autocomplete);
	commands->add_command(context::bus_profile, admin, {F("save")}, bus_profile::save);

	commands->add_command(context::bus_preset, admin, {F("reload")}, bus_preset::reload);
	commands->add_command(context::bus_preset, admin, {F("desc")}, {F("<description>")}, bus_preset::desc, preset_current_description_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("name")}, {F("<name>")}, bus_preset::name, preset_current_name_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("normal")}, bus_preset::normal);
	commands->add_command(context::bus_preset, admin, {F("reverse")}, bus_preset::reverse);
	commands->add_command(context::bus_preset, admin, {F("save")}, {F("[name]")}, bus_preset::save);
	commands->add_command(context::bus_preset, admin, {F("script")}, {F("<script>")}, bus_preset::script, script_names_autocomplete);
	commands->add_command(context::bus_preset, user, {F("show")}, bus_preset::show);
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

void AurcorShell::enter_bus_preset_context(std::shared_ptr<LEDBus> bus, std::shared_ptr<std::shared_ptr<Preset>> preset) {
	if (context() == ShellContext::MAIN) {
		enter_context(ShellContext::BUS_PRESET);
		bus_ = bus;
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
			if (preset_active(false)) {
				text.append(preset_->get()->name().c_str());

				if (preset_->get()->modified())
					text.append("(unsaved)");
			} else {
				text.append("<detached>");
			}

			return text;
		}

	default:
		break;
	}

	return AppShell::context_text();
}

void AurcorShell::set_command(Shell &shell) {
	Config config;

	AppShell::set_command(shell);

	if (shell.has_any_flags(CommandFlags::ADMIN)) {
		shell.printfln(F("Default bus = %s"), config.default_bus().empty() ? "<unset>" : config.default_bus().c_str());
	}
}

bool AurcorShell::preset_active(bool exit) {
	if (preset_) {
		auto preset = to_app(*this).edit(bus_);
		if (preset && *preset == *preset_) {
			return true;
		}

		preset_.reset();
	}

	if (exit) {
		printfln(F("Preset no longer running"));
		exit_context();
	}

	return false;
}

} // namespace aurcor

namespace app {

void setup_commands(std::shared_ptr<Commands> &commands) {
	aurcor::setup_commands(commands);
}

} // namespace app
