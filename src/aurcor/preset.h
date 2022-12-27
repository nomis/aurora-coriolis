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

#pragma once

#include <bitset>
#include <limits>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

extern "C" {
	#include <py/obj.h>
}

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/console.h>
#include <uuid/log.h>

#include "app.h"
#include "script_config.h"
#include "util.h"

namespace aurcor {

class LEDBus;
class MicroPythonFile;

class Preset: public std::enable_shared_from_this<Preset> {
public:
	static constexpr size_t MAX_NAME_LENGTH = 48;
	static constexpr size_t MAX_DESCRIPTION_LENGTH = 48;

	Preset(App &app, std::shared_ptr<LEDBus> bus, std::string name = "");
	~Preset() = default;

	static std::vector<std::string> names();

	std::string name(bool allow_unnamed = false) const;
	bool name(std::string name);

	std::string description() const;
	bool description(std::string description);

	std::string script() const;
	void script(const std::string &script);

	bool reverse() const;
	void reverse(bool reverse);

	void register_config(mp_obj_t dict);
	bool populate_config(mp_obj_t dict);

	std::vector<std::string> config_keys(std::bitset<ScriptConfig::Type::INVALID> types = std::numeric_limits<unsigned long>::max()) const;
	ScriptConfig::Type config_key_type(const std::string &key) const;
	std::vector<std::string> config_container_values(const std::string &key) const;
	Result add_config(const std::string &key, const std::string &value, size_t before = std::numeric_limits<size_t>::max());
	Result move_config(const std::string &key, size_t from_position, size_t to_position);
	Result copy_config(const std::string &key, size_t from_position, size_t to_position);
	Result del_config(const std::string &key, const std::string &value);
	Result del_config(const std::string &key, size_t index);
	Result set_config(const std::string &key, const std::string &value);
	Result unset_config(const std::string &key);
	bool print_config(uuid::console::Shell &shell, const std::string *filter_key = nullptr) const;
	void clear_config();

	size_t config_keys_size() const;
	size_t config_defaults_size() const;
	size_t config_values_size() const;

	/* Not protected by a mutex; assumes modifications only happen from one
	 * thread. Making these thread-safe would require an extra mutex to avoid
	 * blocking other readers when saving the preset.
	 */
	bool editing() const { return !editing_.expired(); }
	std::shared_ptr<std::shared_ptr<Preset>> edit();
	bool modified() const { return modified_; }
	void modified(bool modified) { modified_ = modified; }

	Result load();
	Result save();

	Result rename(const Preset &destination);
	Result remove();

	void loop();

private:
	static constexpr size_t RESTART_TIME_MS = 10000;

	static uuid::log::Logger logger_;

	bool description_constrained(std::string &description);

	std::string make_filename() const;
	bool restart() const;

	void reset();
	Result load(qindesign::cbor::Reader &reader);
	void save(qindesign::cbor::Writer &writer);

	App &app_;
	std::shared_ptr<LEDBus> bus_;
	std::shared_ptr<MicroPythonFile> mp_;
	bool running_{false};
	uint64_t stop_time_ms_{0};

	mutable std::shared_mutex data_mutex_;
	std::string name_;
	std::string description_;
	std::string script_;
	bool script_changed_{false};
	bool reverse_{false};
	ScriptConfig config_;

	std::weak_ptr<std::shared_ptr<Preset>> editing_;
	bool modified_{false};
	bool config_changed_{true};
};

} // namespace aurcor
