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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <uuid/console.h>
#include <uuid/log.h>

#include "aurcor/app.h"
#include "aurcor/console.h"
#include "aurcor/micropython.h"
#include "aurcor/led_profile.h"
#include "aurcor/preset.h"
#include "aurcor/script_config.h"
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

namespace bus_preset_cfgcontainer {
	static void show(Shell &shell, const std::vector<std::string> &arguments);
}

__attribute__((noinline))
static void led_profile_result(AurcorShell &shell, Result result = Result::OK, const __FlashStringHelper *message = nullptr) {
	switch (result) {
	case Result::OK:
		if (message) {
			shell.println(message);
		} else {
			shell.profile().print(shell, shell.bus()->length());
		}
		break;

	case Result::FULL:
		shell.println(F("Profile full"));
		break;

	case Result::OUT_OF_RANGE:
		shell.println(F("Index out of range"));
		break;

	case Result::NOT_FOUND:
		shell.println(F("Index not found"));
		break;

	case Result::PARSE_ERROR:
		shell.println(F("File parse error"));
		break;

	case Result::IO_ERROR:
		shell.println(F("File I/O error"));
		break;
	}
}

__attribute__((noinline))
static bool load_preset(Shell &shell, Preset &preset) {
	switch (preset.load()) {
	case Result::OK:
		return true;

	case Result::NOT_FOUND:
		shell.printfln(F("Preset \"%s\" not found"), preset.name().c_str());
		break;

	case Result::FULL:
		shell.printfln(F("Too many config values in preset \"%s\""), preset.name().c_str());
		break;

	case Result::PARSE_ERROR:
		shell.printfln(F("Parse error loading preset \"%s\""), preset.name().c_str());
		break;

	case Result::OUT_OF_RANGE:
	case Result::IO_ERROR:
	default:
		shell.printfln(F("Error reading preset \"%s\""), preset.name().c_str());
		break;
	}

	return false;
}

__attribute__((noinline))
static void preset_config_result(AurcorShell &shell, const std::string &name,
		const std::string &value, const std::string &position, Result result = Result::OK,
		bool container_value = false, bool ok_container_context = false,
		bool container_position = false) {
	switch (result) {
	case Result::OK:
		if (ok_container_context) {
			bus_preset_cfgcontainer::show(shell, {});
		} else {
			shell.preset().print_config(shell, &name);
		}
		break;

	case Result::FULL:
		shell.println(F("Config full"));
		break;

	case Result::NOT_FOUND:
		if (container_position) {
			shell.printfln(F("Config property \"%s\" or position \"%s\" not found"), name.c_str(), position.c_str());
		} else if (container_value) {
			shell.printfln(F("Config property \"%s\" or value \"%s\" not found"), name.c_str(), value.c_str());
		} else {
			shell.printfln(F("Config property \"%s\" not found"), name.c_str());
		}
		break;

	case Result::OUT_OF_RANGE:
	case Result::PARSE_ERROR:
	case Result::IO_ERROR:
	default:
		shell.printfln(F("Config value \"%s\" invalid for property \"%s\""), value.c_str(), name.c_str());
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

		if (to_shell(shell).profile().get(index, r, g, b) == Result::OK) {
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

	if (aurcor_shell.preset_active()) {
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

	if (aurcor_shell.preset_active()) {
		return {aurcor_shell.preset().description()};
	}

	return {};
}

__attribute__((noinline))
static std::vector<std::string> preset_config_property_name_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto &aurcor_shell = to_shell(shell);

	if (aurcor_shell.preset_active()) {
		auto keys = aurcor_shell.preset().config_keys();
		std::sort(keys.begin(), keys.end());
		return keys;
	}

	return {};
}

__attribute__((noinline))
static std::vector<std::string> preset_config_property_primitive_name_value_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		auto &aurcor_shell = to_shell(shell);

		if (aurcor_shell.preset_active()) {
			ScriptConfig::types_bitset types;
			types[ScriptConfig::Type::BOOL] = true;
			types[ScriptConfig::Type::S32] = true;
			types[ScriptConfig::Type::RGB] = true;

			auto keys = aurcor_shell.preset().config_keys(types);
			std::sort(keys.begin(), keys.end());
			return keys;
		}

		return {};
	} else if (current_arguments.size() == 1) {
		auto &aurcor_shell = to_shell(shell);

		if (aurcor_shell.preset_active()) {
			auto &key = current_arguments[0];
			auto type = aurcor_shell.preset().config_key_type(key);

			switch (type) {
			case ScriptConfig::Type::BOOL:
				return {"true", "false", "t", "f", "1", "0"};

			case ScriptConfig::Type::S32:
			case ScriptConfig::Type::RGB:
			case ScriptConfig::Type::LIST_U16:
			case ScriptConfig::Type::LIST_S32:
			case ScriptConfig::Type::LIST_RGB:
			case ScriptConfig::Type::SET_U16:
			case ScriptConfig::Type::SET_S32:
			case ScriptConfig::Type::SET_RGB:
			case ScriptConfig::Type::INVALID:
				break;
			}
		}

		return {};
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> preset_config_property_container_name_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		auto &aurcor_shell = to_shell(shell);

		if (aurcor_shell.preset_active()) {
			ScriptConfig::types_bitset types;
			types[ScriptConfig::Type::LIST_U16] = true;
			types[ScriptConfig::Type::LIST_S32] = true;
			types[ScriptConfig::Type::LIST_RGB] = true;
			types[ScriptConfig::Type::SET_U16] = true;
			types[ScriptConfig::Type::SET_S32] = true;
			types[ScriptConfig::Type::SET_RGB] = true;

			auto keys = aurcor_shell.preset().config_keys(types);
			std::sort(keys.begin(), keys.end());
			return keys;
		}

		return {};
	} else {
		return {};
	}
}

__attribute__((noinline))
static std::vector<std::string> preset_config_property_container_value_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	auto &aurcor_shell = to_shell(shell);

	if (aurcor_shell.preset_active()) {
		return aurcor_shell.preset().config_container_values(aurcor_shell.preset_cfg_name());
	}

	return {};
}

__attribute__((noinline))
static std::vector<std::string> preset_config_property_container_name_value_autocomplete(Shell &shell,
		const std::vector<std::string> &current_arguments,
		const std::string &next_argument) {
	if (current_arguments.size() == 0) {
		return preset_config_property_container_name_autocomplete(shell, current_arguments, next_argument);
	} else if (current_arguments.size() == 1) {
		auto &aurcor_shell = to_shell(shell);

		if (aurcor_shell.preset_active()) {
			auto &key = current_arguments[0];
			return aurcor_shell.preset().config_container_values(key);
		}

		return {};
	} else {
		return {};
	}
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

/* [bus] */
static void clear(Shell &shell, const std::vector<std::string> &arguments) {
	auto bus = lookup_bus_or_default(shell, arguments, 0);

	if (bus) {
		shell.logger().info("Clearing bus \"%s\" from console %s", bus->name(), to_shell(shell).console_name().c_str());
		bus->clear();
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

			if (!load_preset(shell, *preset))
				return;
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

		if (preset->name(preset_name) && preset->load() == Result::OK) {
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

	switch (preset_from->rename(*preset_to)) {
	case Result::OK:
		app.renamed(preset_from_name, preset_to_name);
		break;

	case Result::NOT_FOUND:
		shell.printfln(F("Preset \"%s\" not found"), preset_from_name.c_str());
		break;

	case Result::FULL:
	case Result::OUT_OF_RANGE:
	case Result::PARSE_ERROR:
	case Result::IO_ERROR:
		shell.printfln(F("Error renaming preset"));
		break;
	}
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

	switch (preset->remove()) {
	case Result::OK:
		app.deleted(preset_name);
		break;

	case Result::NOT_FOUND:
		shell.printfln(F("Preset \"%s\" not found"), preset_name.c_str());
		break;

	case Result::FULL:
	case Result::OUT_OF_RANGE:
	case Result::PARSE_ERROR:
	case Result::IO_ERROR:
	default:
		shell.printfln(F("Error deleting preset"));
		break;
	}

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

		if (load_preset(shell, *preset)) {
			app.start(bus, preset);

			if (arguments.size() >= 3 && arguments[2] == "default") {
				if (shell.has_any_flags(CommandFlags::ADMIN)) {
					bus->default_preset(preset_name);
					aurcor::console::bus::show_default_preset(shell, bus);
				}
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

		shell.block_with([bus] (Shell &shell, bool stop) mutable -> bool {
			return to_app(shell).detach(bus, nullptr, true);
		});
	}
}

} // namespace main

namespace bus {

__attribute__((noinline))
static void show_length(Shell &shell) {
	shell.printfln(F("Length:         %zu"), to_shell(shell).bus()->length());
};

__attribute__((noinline))
static void show_direction(Shell &shell) {
	shell.printfln(F("Direction:      %s"), to_shell(shell).bus()->reverse() ? "reverse" : "normal");
};

__attribute__((noinline))
static void show_default_preset(Shell &shell, std::shared_ptr<LEDBus> &bus) {
	auto default_preset = bus->default_preset();
	shell.printfln(F("Default preset: %s"), default_preset.empty() ? "<unset>" : default_preset.c_str());
};

static void clear(Shell &shell, const std::vector<std::string> &arguments) {
	auto &bus = to_shell(shell).bus();

	shell.logger().info("Clearing bus \"%s\" from console %s", bus->name(), to_shell(shell).console_name().c_str());
	bus->clear();
}

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

			if (!load_preset(shell, *preset))
				return;
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

		if (load_preset(shell, *preset)) {
			app.start(bus, preset);
		} else {
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

	if (load_preset(shell, *preset)) {
		app.start(bus, preset);

		if (arguments.size() >= 2 && arguments[1] == "default") {
			if (shell.has_any_flags(CommandFlags::ADMIN)) {
				bus->default_preset(preset_name);
				show_default_preset(shell, bus);
			}
		}
	}
}

static void stop(Shell &shell, const std::vector<std::string> &arguments) {
	auto &app = to_app(shell);
	auto &bus = to_shell(shell).bus();

	app.stop(bus);

	shell.block_with([bus] (Shell &shell, bool stop) mutable -> bool {
		return to_app(shell).detach(bus, nullptr, true);
	});
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	show_length(shell);
	show_direction(shell);
	show_default_preset(shell, to_shell(shell).bus());

	auto preset = to_app(shell).edit(to_shell(shell).bus());

	shell.printfln(F("Current preset: %s%s"),
		preset ? preset->get()->name().c_str() : "<none>",
		preset ? (preset->get()->modified() ? " (unsaved)" : "") : "");
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

__attribute__((noinline))
static void show_name(Shell &shell) {
	auto &aurcor_shell = to_shell(shell);
	auto &preset = aurcor_shell.preset();

	shell.printfln(F("Name:        %s"), preset.name().c_str());
};

__attribute__((noinline))
static void show_description(Shell &shell) {
	auto &aurcor_shell = to_shell(shell);
	auto &preset = aurcor_shell.preset();

	shell.printfln(F("Description: %s"), preset.description().c_str());
};

__attribute__((noinline))
static void show_script(Shell &shell) {
	auto &aurcor_shell = to_shell(shell);
	auto &preset = aurcor_shell.preset();

	shell.printfln(F("Script:      %s"), preset.script().c_str());
};

__attribute__((noinline))
static void show_direction(Shell &shell) {
	auto &aurcor_shell = to_shell(shell);
	auto &preset = aurcor_shell.preset();

	shell.printfln(F("Direction:   %s"), preset.reverse() ? "reverse" : "normal");
};

/* <config property> <value> */
static void add(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &value = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().add_config(name, value));
}

/* <config property> <value> */
static void del(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &value = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().del_config(name, value),
		true);
}

/* <description> */
static void desc(Shell &shell, const std::vector<std::string> &arguments) {
	auto &description = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!aurcor_shell.preset().description(description))
		shell.printfln(F("Invalid description"));

	show_description(shell);
}

/* <config property> */
static void edit(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto result = aurcor_shell.preset().config_key_type(name);

	switch (result) {
	case ScriptConfig::Type::LIST_U16:
	case ScriptConfig::Type::LIST_S32:
	case ScriptConfig::Type::LIST_RGB:
		aurcor_shell.enter_bus_preset_cfglist_context(name);
		bus_preset_cfgcontainer::show(shell, {});
		break;

	case ScriptConfig::Type::SET_U16:
	case ScriptConfig::Type::SET_S32:
	case ScriptConfig::Type::SET_RGB:
		aurcor_shell.enter_bus_preset_cfgset_context(name);
		bus_preset_cfgcontainer::show(shell, {});
		break;

	case ScriptConfig::Type::BOOL:
	case ScriptConfig::Type::S32:
	case ScriptConfig::Type::RGB:
		shell.printfln(F("Config property \"%s\" is not a list or a set"), name.c_str());
		break;

	case ScriptConfig::Type::INVALID:
		shell.printfln(F("Config property \"%s\" not found"), name.c_str());
		break;
	}
}

static void reload(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (load_preset(shell, aurcor_shell.preset()))
		shell.printfln(F("Reloaded preset"));
}

static void reset_config(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	aurcor_shell.preset().clear_config();
	shell.printfln(F("Reset config to defaults"));
}

/* <name> */
static void name(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (!aurcor_shell.preset().name(name))
		shell.printfln(F("Invalid name"));

	show_name(shell);
}

static void normal(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	aurcor_shell.preset().reverse(false);
	show_direction(shell);
}

static void reverse(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	aurcor_shell.preset().reverse(true);
	show_direction(shell);
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

	if (aurcor_shell.preset().save() == Result::OK) {
		to_app(shell).refresh(aurcor_shell.preset().name());
	} else {
		shell.printfln(F("Failed to save preset"));
	}
}

/* <script> */
static void script(Shell &shell, const std::vector<std::string> &arguments) {
	auto &script_name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	if (MicroPythonFile::exists(script_name.c_str())) {
		aurcor_shell.preset().script(script_name);
		show_script(shell);
	} else {
		shell.printfln(F("Script \"%s\" not found"), script_name.c_str());
	}
}

/* <config property> <value> */
static void set(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &value = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().set_config(name, value));
}

/* <config property> */
static void unset(Shell &shell, const std::vector<std::string> &arguments) {
	auto &name = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	preset_config_result(aurcor_shell, name, "", "",
		aurcor_shell.preset().unset_config(name));
}

/* [config property] */
static void show(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &preset = aurcor_shell.preset();

	if (arguments.empty()) {
		show_name(shell);
		show_description(shell);
		show_script(shell);
		show_direction(shell);

		auto keys_size = preset.config_keys_size();
		auto defaults_size = preset.config_defaults_size();
		auto values_size = preset.config_values_size();

		shell.printfln(F("Config size:"));
		shell.printfln(F("             %3zu%%   (keys)   %4zu/%4zu"),
			keys_size * 100U / std::max(ScriptConfig::MAX_DEFAULTS_SIZE, ScriptConfig::MAX_VALUES_SIZE),
			keys_size, std::max(ScriptConfig::MAX_DEFAULTS_SIZE, ScriptConfig::MAX_VALUES_SIZE));
		shell.printfln(F("             %3zu%% (defaults) %4zu/%4zu"),
			defaults_size * 100U / ScriptConfig::MAX_DEFAULTS_SIZE,
			defaults_size, ScriptConfig::MAX_DEFAULTS_SIZE);
		shell.printfln(F("             %3zu%%  (values)  %4zu/%4zu"),
			values_size * 100U / ScriptConfig::MAX_VALUES_SIZE,
			values_size, ScriptConfig::MAX_VALUES_SIZE);
		shell.println();
		preset.print_config(shell);
	} else {
		auto &property_name = arguments[0];

		if (!preset.print_config(shell, &property_name)) {
			shell.printfln(F("Config property \"%s\" not found"), property_name.c_str());
		}
	}
}

} // namespace bus_preset

namespace bus_preset_cfgcontainer {

/* <value> */
static void add(Shell &shell, const std::vector<std::string> &arguments) {
	auto &value = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().add_config(name, value),
		false, true);
}

static void clear(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, "", "",
		aurcor_shell.preset().unset_config(name),
		false, true);
}

static void show(Shell &shell, const std::vector<std::string> &arguments) {
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &property_name = aurcor_shell.preset_cfg_name();
	auto values = aurcor_shell.preset().config_container_values(property_name);

	if (values.empty()) {
		shell.println(F("No values"));
	} else {
		size_t index_len = std::to_string(values.size() - 1).length();
		size_t value_len = 1;
		size_t i = 0;

		for (auto &value : values) {
			value_len = std::max(value_len, value.length());
		}

		for (auto &value : values) {
			shell.printfln(F("%*zu: %*s"), index_len, i, value_len, value.c_str());
			i++;
		}
	}
}

} // namespace bus_preset_cfgcontainer

namespace bus_preset_cfglist {

static bool parse_position(Shell &shell, const std::string &text, size_t &value) {
	char *end;
	long long ll_value = std::strtoll(text.c_str(), &end, 0);
	bool ret;

	if (std::is_same<typeof(ll_value), typeof(value)>::value
			&& (ll_value < std::numeric_limits<typeof(value)>::min()
				|| ll_value > std::numeric_limits<typeof(value)>::max())) {
		ret = false;
	} else {
		value = ll_value;
		ret = end[0] == '\0';
	}

	if (!ret)
		shell.printfln(F("Position \"%s\" invalid"), text.c_str());

	return ret;
}

/* <position> <value> */
static void after(Shell &shell, const std::vector<std::string> &arguments) {
	auto &position = arguments[0];
	auto &value = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	size_t int_position;

	if (!parse_position(shell, position, int_position))
		return;

	if (int_position != std::numeric_limits<typeof(int_position)>::max())
		int_position++;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().add_config(name, value, int_position),
		false, true);
}

/* <position> <value> */
static void before(Shell &shell, const std::vector<std::string> &arguments) {
	auto &position = arguments[0];
	auto &value = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	size_t int_position;

	if (!parse_position(shell, position, int_position))
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().add_config(name, value, int_position),
		false, true);
}

/* <position> <position> */
static void cp(Shell &shell, const std::vector<std::string> &arguments) {
	auto &from_position = arguments[0];
	auto &to_position = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	size_t int_from_position;
	size_t int_to_position;

	if (!parse_position(shell, from_position, int_from_position))
		return;

	if (!parse_position(shell, to_position, int_to_position))
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, "", from_position,
		aurcor_shell.preset().copy_config(name, int_from_position, int_to_position),
		false, true, true);
}

/* <position> <position> */
static void mv(Shell &shell, const std::vector<std::string> &arguments) {
	auto &from_position = arguments[0];
	auto &to_position = arguments[1];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	size_t int_from_position;
	size_t int_to_position;

	if (!parse_position(shell, from_position, int_from_position))
		return;

	if (!parse_position(shell, to_position, int_to_position))
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, "", from_position,
		aurcor_shell.preset().move_config(name, int_from_position, int_to_position),
		false, true, true);
}

/* <value> */
static void prepend(Shell &shell, const std::vector<std::string> &arguments) {
	auto &value = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().add_config(name, value, 0),
		false, true);
}

/* <position> */
static void rm(Shell &shell, const std::vector<std::string> &arguments) {
	auto &position = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	size_t int_position;

	if (!parse_position(shell, position, int_position))
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, "", position,
		aurcor_shell.preset().del_config(name, int_position),
		false, true);
}

} // namespace bus_preset_cfglist

namespace bus_preset_cfgset {

/* <value> */
static void del(Shell &shell, const std::vector<std::string> &arguments) {
	auto &value = arguments[0];
	auto &aurcor_shell = to_shell(shell);

	if (!aurcor_shell.preset_active())
		return;

	auto &name = aurcor_shell.preset_cfg_name();
	preset_config_result(aurcor_shell, name, value, "",
		aurcor_shell.preset().del_config(name, value),
		true, true);
}

} // namespace bus_preset_cfgset

namespace context {

static constexpr auto main = ShellContext::MAIN;
static constexpr auto bus = ShellContext::BUS;
static constexpr auto bus_profile = ShellContext::BUS_PROFILE;
static constexpr auto bus_preset = ShellContext::BUS_PRESET;
static constexpr auto bus_preset_cfglist = ShellContext::BUS_PRESET_CFGLIST;
static constexpr auto bus_preset_cfgset = ShellContext::BUS_PRESET_CFGSET;

} // namespace context

static constexpr auto user = CommandFlags::USER;
static constexpr auto admin = CommandFlags::ADMIN;

} // namespace console

using namespace console;

static inline void setup_commands(std::shared_ptr<Commands> &commands) {
	commands->add_command(context::main, user, {F("bus")}, {F("[bus]")}, main::bus, bus_names_autocomplete);
	commands->add_command(context::main, user, {F("clear")}, {F("[bus]")}, main::clear, bus_names_autocomplete);
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
	commands->add_command(context::bus, user, {F("clear")}, bus::clear);
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

	commands->add_command(context::bus_preset, admin, {F("add")}, {F("<config property>"), F("<value>")}, bus_preset::add, preset_config_property_container_name_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("del")}, {F("<config property>"), F("<value>")}, bus_preset::del, preset_config_property_container_name_value_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("desc")}, {F("<description>")}, bus_preset::desc, preset_current_description_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("edit")}, {F("<config property>")}, bus_preset::edit, preset_config_property_container_name_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("name")}, {F("<name>")}, bus_preset::name, preset_current_name_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("normal")}, bus_preset::normal);
	commands->add_command(context::bus_preset, admin, {F("reload")}, bus_preset::reload);
	commands->add_command(context::bus_preset, admin, {F("reset"), F("config")}, bus_preset::reset_config);
	commands->add_command(context::bus_preset, admin, {F("reverse")}, bus_preset::reverse);
	commands->add_command(context::bus_preset, admin, {F("save")}, {F("[name]")}, bus_preset::save);
	commands->add_command(context::bus_preset, admin, {F("script")}, {F("<script>")}, bus_preset::script, script_names_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("set")}, {F("<config property>"), F("<value>")}, bus_preset::set, preset_config_property_primitive_name_value_autocomplete);
	commands->add_command(context::bus_preset, user, {F("show")}, {F("[config property]")}, bus_preset::show, preset_config_property_name_autocomplete);
	commands->add_command(context::bus_preset, admin, {F("unset")}, {F("<config property>")}, bus_preset::unset, preset_config_property_name_autocomplete);

	commands->add_command(context::bus_preset_cfglist, admin, {F("after")}, {F("<position>"), F("<value>")}, bus_preset_cfglist::after);
	commands->add_command(context::bus_preset_cfglist, admin, {F("append")}, {F("<value>")}, bus_preset_cfgcontainer::add);
	commands->add_command(context::bus_preset_cfglist, admin, {F("before")}, {F("<position>"), F("<value>")}, bus_preset_cfglist::before);
	commands->add_command(context::bus_preset_cfglist, admin, {F("clear")}, bus_preset_cfgcontainer::clear);
	commands->add_command(context::bus_preset_cfglist, admin, {F("cp")}, {F("<position>"), F("<position>")}, bus_preset_cfglist::cp);
	commands->add_command(context::bus_preset_cfglist, admin, {F("mv")}, {F("<position>"), F("<position>")}, bus_preset_cfglist::mv);
	commands->add_command(context::bus_preset_cfglist, admin, {F("prepend")}, {F("<value>")}, bus_preset_cfglist::prepend);
	commands->add_command(context::bus_preset_cfglist, admin, {F("rm")}, {F("<position>")}, bus_preset_cfglist::rm);
	commands->add_command(context::bus_preset_cfglist, user, {F("show")}, bus_preset_cfgcontainer::show);

	commands->add_command(context::bus_preset_cfgset, admin, {F("add")}, {F("<value>")}, bus_preset_cfgcontainer::add);
	commands->add_command(context::bus_preset_cfgset, admin, {F("clear")}, bus_preset_cfgcontainer::clear);
	commands->add_command(context::bus_preset_cfgset, admin, {F("del")}, {F("<value>")}, bus_preset_cfgset::del, preset_config_property_container_value_autocomplete);
	commands->add_command(context::bus_preset_cfgset, user, {F("show")}, bus_preset_cfgcontainer::show);
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

void AurcorShell::enter_bus_preset_cfglist_context(const std::string &name) {
	if (context() == ShellContext::BUS_PRESET) {
		enter_context(ShellContext::BUS_PRESET_CFGLIST);
		preset_cfg_name_ = name;
	}
}

void AurcorShell::enter_bus_preset_cfgset_context(const std::string &name) {
	if (context() == ShellContext::BUS_PRESET) {
		enter_context(ShellContext::BUS_PRESET_CFGSET);
		preset_cfg_name_ = name;
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
	if (prev_context == ShellContext::BUS_PRESET_CFGLIST
			|| prev_context == ShellContext::BUS_PRESET_CFGSET
			|| new_context == ShellContext::MAIN) {
		preset_cfg_name_ = "";
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
				text.append(" (unsaved)");

			return text;
		}

	case ShellContext::BUS_PRESET: {
			std::string text{"/bus/"};

			text.append(bus_->name());

			text.append("/preset/");
			if (preset_active(false)) {
				text.append(preset_->get()->name().c_str());

				if (preset_->get()->modified())
					text.append(" (unsaved)");
			} else {
				text.append("<detached>");
			}

			return text;
		}

	case ShellContext::BUS_PRESET_CFGLIST:
	case ShellContext::BUS_PRESET_CFGSET: {
			std::string text{"/bus/"};

			text.append(bus_->name());

			text.append("/preset/");
			if (preset_active(false)) {
				text.append(preset_->get()->name().c_str());
			} else {
				text.append("<detached>");
			}

			text.append("/config/");
			if (preset_active(false)) {
				text.append(preset_cfg_name_);
			} else {
				text.append("<detached>");
			}

			return text;
		}

	case ShellContext::MAIN:
	case ShellContext::FILESYSTEM:
	case ShellContext::END:
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
