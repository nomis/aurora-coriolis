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

#include "aurcor/script_config.h"

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

extern "C" {
	#include <py/misc.h>
	#include <py/obj.h>
	#include <py/objlist.h>
	#include <py/objstr.h>
	#include <py/runtime.h>
}

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/console.h>
#include <uuid/log.h>

#include "app/util.h"
#include "aurcor/micropython.h"
#include "aurcor/preset.h"
#include "aurcor/util.h"

namespace cbor = qindesign::cbor;
using uuid::console::Shell;

static const char __pstr__logger_name[] __attribute__((__aligned__(PSTR_ALIGN))) PROGMEM = "script-config";

namespace aurcor {

uuid::log::Logger ScriptConfig::logger_{FPSTR(__pstr__logger_name), uuid::log::Facility::DAEMON};

inline ScriptConfig::Property::pointer_type ScriptConfig::Property::create(ScriptConfig::Type type, bool registered) {
	switch (type) {
	case Type::BOOL:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::BoolProperty(registered)};

	case Type::S32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::S32Property(false, registered)};

	case Type::RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::S32Property(true, registered)};

	case Type::LIST_U16:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListU16Property(registered)};

	case Type::LIST_S32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListS32Property(false, registered)};

	case Type::LIST_RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListS32Property(true, registered)};

	case Type::SET_U16:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetU16Property(registered)};

	case Type::SET_S32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetS32Property(false, registered)};

	case Type::SET_RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetS32Property(true, registered)};

	case Type::INVALID:
	default:
		return {nullptr};
	}
}

void ScriptConfig::Property::Deleter::operator()(Property *property) {
	switch (property->type()) {
	case Type::BOOL:
		delete static_cast<ScriptConfig::BoolProperty*>(property);
		break;

	case Type::S32:
		delete static_cast<ScriptConfig::S32Property*>(property);
		break;

	case Type::RGB:
		delete static_cast<ScriptConfig::S32Property*>(property);
		break;

	case Type::LIST_U16:
		delete static_cast<ScriptConfig::ListU16Property*>(property);
		break;

	case Type::LIST_S32:
		delete static_cast<ScriptConfig::ListS32Property*>(property);
		break;

	case Type::LIST_RGB:
		delete static_cast<ScriptConfig::ListS32Property*>(property);
		break;

	case Type::SET_U16:
		delete static_cast<ScriptConfig::SetU16Property*>(property);
		break;

	case Type::SET_S32:
		delete static_cast<ScriptConfig::SetS32Property*>(property);
		break;

	case Type::SET_RGB:
		delete static_cast<ScriptConfig::SetS32Property*>(property);
		break;

	case Type::INVALID:
	default:
		delete property;
		break;
	}
}

size_t ScriptConfig::Property::size(bool values) const {
	switch (type()) {
	case Type::BOOL:
		return as_bool().size(values);

	case Type::S32:
	case Type::RGB:
		return as_s32().size(values);

	case Type::LIST_U16:
		return as_s32_list().size(values);

	case Type::LIST_S32:
	case Type::LIST_RGB:
		return as_s32_list().size(values);

	case Type::SET_U16:
		return as_u16_set().size(values);

	case Type::SET_S32:
	case Type::SET_RGB:
		return as_s32_set().size(values);

	case Type::INVALID:
	default:
		return 0;
	}
}

bool ScriptConfig::Property::has_value(Property &property) {
	switch (property.type()) {
	case Type::BOOL:
		return property.as_bool().has_value();

	case Type::S32:
	case Type::RGB:
		return property.as_s32().has_value();

	case Type::LIST_U16:
		return property.as_s32_list().has_value();

	case Type::LIST_S32:
	case Type::LIST_RGB:
		return property.as_s32_list().has_value();

	case Type::SET_U16:
		return property.as_u16_set().has_value();

	case Type::SET_S32:
	case Type::SET_RGB:
		return property.as_s32_set().has_value();

	case Type::INVALID:
	default:
		return false;
	}
}

bool ScriptConfig::Property::clear_default(Property &property) {
	switch (property.type()) {
	case Type::BOOL:
		property.as_bool().clear_default();
		return property.as_bool().has_value();

	case Type::S32:
	case Type::RGB:
		property.as_s32().clear_default();
		return property.as_s32().has_value();

	case Type::LIST_U16:
		property.as_s32_list().clear_default();
		return property.as_s32_list().has_value();

	case Type::LIST_S32:
	case Type::LIST_RGB:
		property.as_s32_list().clear_default();
		return property.as_s32_list().has_value();

	case Type::SET_U16:
		property.as_u16_set().clear_default();
		return property.as_u16_set().has_value();

	case Type::SET_S32:
	case Type::SET_RGB:
		property.as_s32_set().clear_default();
		return property.as_s32_set().has_value();

	case Type::INVALID:
	default:
		return false;
	}
}

bool ScriptConfig::Property::clear_value(Property &property) {
	switch (property.type()) {
	case Type::BOOL:
		property.as_bool().clear_value();
		return property.as_bool().registered();

	case Type::S32:
	case Type::RGB:
		property.as_s32().clear_value();
		return property.as_s32().registered();

	case Type::LIST_U16:
		property.as_s32_list().clear_value();
		return property.as_s32_list().registered();

	case Type::LIST_S32:
	case Type::LIST_RGB:
		property.as_s32_list().clear_value();
		return property.as_s32_list().registered();

	case Type::SET_U16:
		property.as_u16_set().clear_value();
		return property.as_u16_set().registered();

	case Type::SET_S32:
	case Type::SET_RGB:
		property.as_s32_set().clear_value();
		return property.as_s32_set().registered();

	case Type::INVALID:
	default:
		return false;
	}
}

ScriptConfig::Type ScriptConfig::type_of(const std::string &type) {
	if (type == "bool") {
		return Type::BOOL;
	} else if (type == "s32") {
		return Type::S32;
	} else if (type == "rgb") {
		return Type::RGB;
	} else if (type == "list_u16") {
		return Type::LIST_U16;
	} else if (type == "list_s32") {
		return Type::LIST_S32;
	} else if (type == "list_rgb") {
		return Type::LIST_RGB;
	} else if (type == "set_u16") {
		return Type::SET_U16;
	} else if (type == "set_s32") {
		return Type::SET_S32;
	} else if (type == "set_rgb") {
		return Type::SET_RGB;
	} else {
		return Type::INVALID;
	}
}

inline size_t ScriptConfig::entry_base_size(const std::string &key) {
	return rounded_sizeof<typeof(key)>()
			+ key.length() + 1
			+ rounded_sizeof<ScriptConfig::Property::pointer_type>();
}

inline size_t ScriptConfig::entry_size(const std::string &key, const ScriptConfig::Property &value, bool values) {
	return entry_base_size(key) + value.size(values);
}

size_t ScriptConfig::size(bool values) const {
	size_t total = 0;

	for (auto &entry : properties_) {
		total += entry_size(entry.first, *entry.second, values);
	}

	return total;
}

bool ScriptConfig::allowed_key(const std::string &key) {
	return !key.empty() && key.length() < MAX_KEY_LENGTH && allowed_file_name(key);
}

mp_int_t ScriptConfig::convert_rgb_value(mp_obj_t value_obj) {
	if (mp_obj_is_int(value_obj)) {
		return mp_obj_get_int(value_obj);
	} else {
		int32_t value = 0;

		for (size_t i = 0; i < 3; i++) {
			value <<= 8;
			value |= int_to_u8(mp_obj_get_int(mp_obj_subscr(value_obj, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL)));
		}

		return value;
	}
}

template <class T>
void ScriptConfig::convert_container_values(mp_obj_t container_obj, mp_int_t (*convert_value)(mp_obj_t value_obj), T &property, size_t total_size) {
	mp_obj_t iterable = mp_getiter(container_obj, NULL);
    mp_obj_t value_obj;
	auto &container = property.defaults();

    while ((value_obj = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
		container::add(container, convert_value(value_obj));

		if (total_size + property.size(false) > MAX_DEFAULTS_SIZE)
			mp_raise_ValueError(MP_ERROR_TEXT("maximum config size exceeded"));
	}
}

template <class T>
mp_obj_t ScriptConfig::create_list(const std::vector<T> &container) {
	mp_obj_list_t *list = m_new_obj(mp_obj_list_t);

	mp_obj_list_init(list, container.size());

	for (size_t i = 0; i < container.size(); i++) {
		list->items[i] = mp_obj_new_int(container[i]);
	}

	return MP_OBJ_FROM_PTR(list);
}

template <class T>
mp_obj_t ScriptConfig::create_set(const std::set<T> &container, typename std::set<T>::const_iterator &container_it) {
	mp_obj_t set = mp_obj_new_set(0, nullptr);

	for (container_it = container.begin(); container_it != container.end(); ++container_it) {
        mp_obj_set_store(set, mp_obj_new_int(*container_it));
	}

	return set;
}

void ScriptConfig::register_properties(mp_obj_t dict) {
	micropython_nlr_begin();

	std::string key;
	auto property_it = properties_.end();

	micropython_nlr_try();
	if (mp_obj_get_type(dict) != &mp_type_dict)
		mp_raise_TypeError(MP_ERROR_TEXT("parameter must be a dict"));

	mp_map_t *map = mp_obj_dict_get_map(dict);

	/*
	 * Delete keys that have been removed or have changed type and clear all
	 * default values before re-populating them. This ensures that the size is
	 * calculated based on empty values so it won't be exceeded if a large set
	 * of default values is moved from a later property to an earlier one.
	 */
	for (property_it = properties_.begin(); property_it != properties_.end(); ) {
		qstr qkey = qstr_from_strn(property_it->first.c_str(), property_it->first.length());
		mp_map_elem_t *elem = mp_map_lookup(map, MP_OBJ_NEW_QSTR(qkey), MP_MAP_LOOKUP);

		if (elem == MP_OBJ_NULL) {
			property_it->second->registered(false);
			if (!Property::clear_default(*property_it->second)) {
				property_it = properties_.erase(property_it);
				continue;
			}
		} else {
			Type type = type_of(mp_obj_str_get_str(mp_obj_subscr(elem->value, MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_SENTINEL)));

			if (type != Type::INVALID && property_it->second->type() != type) {
				property_it = properties_.erase(property_it);
				continue;
			}

			Property::clear_default(*property_it->second);
		}

		++property_it;
	}

	size_t total_size = defaults_size();

	for (size_t i = 0; i < map->alloc; i++) {
        if (!mp_map_slot_is_filled(map, i))
			continue;

		auto &key_obj = map->table[i].key;
		auto &value_obj = map->table[i].value;

		Type type = type_of(mp_obj_str_get_str(mp_obj_subscr(value_obj, MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_SENTINEL)));
		mp_obj_t default_value = mp_obj_subscr(value_obj, MP_OBJ_NEW_SMALL_INT(1), MP_OBJ_SENTINEL);

		key = mp_obj_str_get_str(key_obj);
		if (!allowed_key(key))
			mp_raise_ValueError(MP_ERROR_TEXT("invalid key"));
		property_it = properties_.find(key);

		if (type == Type::INVALID)
			mp_raise_ValueError(MP_ERROR_TEXT("invalid config type"));

		if (property_it != properties_.end()) {
			property_it->second->registered(true);
			total_size -= property_it->second->size(false);
		} else {
			property_it = properties_.emplace(key, std::move(ScriptConfig::Property::create(type, true))).first;
			total_size += entry_base_size(key);
		}

		ScriptConfig::Property &property = *property_it->second;
		auto *mp_obj_get_int_not_const = reinterpret_cast<mp_int_t (*)(mp_obj_t)>(&mp_obj_get_int);

		/* Existing properties have already had the default cleared */

		if (total_size + property.size(false) > MAX_DEFAULTS_SIZE)
			mp_raise_ValueError(MP_ERROR_TEXT("maximum config size exceeded"));

		if (default_value != mp_const_none) {
			switch (type) {
			case Type::BOOL:
				property.as_bool().set_default(mp_obj_is_true(default_value));
				break;

			case Type::S32:
				property.as_s32().set_default(mp_obj_get_int(default_value));
				break;

			case Type::RGB:
				property.as_s32().set_default(convert_rgb_value(default_value));
				break;

			case Type::LIST_U16:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_u16_list(), total_size);
				break;

			case Type::LIST_S32:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_s32_list(), total_size);
				break;

			case Type::LIST_RGB:
				convert_container_values(default_value, convert_rgb_value, property.as_s32_list(), total_size);
				break;

			case Type::SET_U16:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_u16_set(), total_size);
				break;

			case Type::SET_S32:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_s32_set(), total_size);
				break;

			case Type::SET_RGB:
				convert_container_values(default_value, convert_rgb_value, property.as_s32_set(), total_size);
				break;

			case Type::INVALID:
				break;
			}
		}

		total_size += property.size(false);
	}

	micropython_nlr_finally();
	micropython_nlr_end();
}

void ScriptConfig::populate_dict(mp_obj_t dict) {
	micropython_nlr_begin();

	auto property_it = properties_.end();
	std::set<uint16_t>::const_iterator set_uint16_it;
	std::set<int32_t>::const_iterator set_int32_it;

	micropython_nlr_try();

	if (mp_obj_get_type(dict) != &mp_type_dict)
		mp_raise_TypeError(MP_ERROR_TEXT("parameter must be a dict"));

	mp_map_t *map = mp_obj_dict_get_map(dict);

	for (property_it = properties_.begin(); property_it != properties_.end(); ++property_it) {
		qstr qkey = qstr_from_strn(property_it->first.c_str(), property_it->first.length());
		mp_map_elem_t *elem = mp_map_lookup(map, MP_OBJ_NEW_QSTR(qkey), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
		ScriptConfig::Property &property = *property_it->second;

		if (!property.registered())
			continue;

		switch (property.type()) {
		case Type::BOOL:
			if (property.as_bool().has_any()) {
				elem->value = mp_obj_new_bool(property.as_bool().get_any());
			} else {
				elem->value = mp_const_none;
			}
			break;

		case Type::S32:
		case Type::RGB:
			if (property.as_s32().has_any()) {
				elem->value = MP_OBJ_NEW_SMALL_INT(property.as_s32().get_any());
			} else {
				elem->value = mp_const_none;
			}
			break;

		case Type::LIST_U16:
			elem->value = create_list(property.as_u16_list().get_any());
			break;

		case Type::LIST_S32:
		case Type::LIST_RGB:
			elem->value = create_list(property.as_s32_list().get_any());
			break;

		case Type::SET_U16:
			elem->value = create_set(property.as_u16_set().get_any(), set_uint16_it);
			break;

		case Type::SET_S32:
		case Type::SET_RGB:
			elem->value = create_set(property.as_s32_set().get_any(), set_int32_it);
			break;

		case Type::INVALID:
			break;
		}
	}

	micropython_nlr_finally();
	micropython_nlr_end();
}

std::vector<std::string> ScriptConfig::keys(types_bitset types) const {
	std::vector<std::string> keys;

	for (auto &entry : properties_) {
		auto &key = entry.first;
		auto &property = *entry.second;

		if (types[property.type()])
			keys.push_back(key);
	}

	return keys;
}

std::vector<std::string> ScriptConfig::filtered_keys(const std::string *filter_key, size_t &max_key_length) const {
	std::vector<std::string> keys;

	for (auto &entry : properties_) {
		auto &key = entry.first;

		if (filter_key && key != *filter_key)
			continue;

		max_key_length = std::max(max_key_length, key.length());
		keys.push_back(key);
	}

	return keys;
}

ScriptConfig::Type ScriptConfig::key_type(const std::string &key) const {
	auto it = properties_.find(key);

	if (it == properties_.end())
		return Type::INVALID;

	return it->second->type();
}

template <class T>
void ScriptConfig::container_values(const T &property, const char *fmt, std::vector<std::string> &values) {
	for (auto &value : property.values()) {
		std::vector<char> text(12);
		std::snprintf(text.data(), text.size(), fmt, value);
		values.emplace_back(text.data());
	}
}

std::vector<std::string> ScriptConfig::container_values(const std::string &key) const {
	std::vector<std::string> values;
	auto it = properties_.find(key);

	if (it != properties_.end()) {
		auto &property = *it->second;

		switch (property.type()) {
		case Type::LIST_U16:
			container_values(property.as_u16_list(), "%u", values);
			break;

		case Type::LIST_S32:
			container_values(property.as_s32_list(), "%d", values);
			break;

		case Type::LIST_RGB:
			container_values(property.as_s32_list(), "#%06X", values);
			break;

		case Type::SET_U16:
			container_values(property.as_u16_set(), "%u", values);
			break;

		case Type::SET_S32:
			container_values(property.as_s32_set(), "%d", values);
			break;

		case Type::SET_RGB:
			container_values(property.as_s32_set(), "#%06X", values);
			break;

		case Type::BOOL:
		case Type::S32:
		case Type::RGB:
		case Type::INVALID:
			break;
		}
	}

	return values;
}

bool ScriptConfig::parse_u16(ContainerOp op, const std::string &text, uint16_t &value) {
	switch (op) {
	case ContainerOp::ADD:
	case ContainerOp::DEL_VALUE:
		break;

	case ContainerOp::DEL_POSITION:
	case ContainerOp::MOVE_POSITION:
	case ContainerOp::COPY_POSITION:
	default:
		return true;
	}

	char *end;
	long long ll_value = std::strtoll(text.c_str(), &end, 0);

	if (text.empty()
			|| ll_value < std::numeric_limits<typeof(value)>::min()
			|| ll_value > std::numeric_limits<typeof(value)>::max())
		return false;

	value = ll_value;
	return end[0] == '\0';
}

bool ScriptConfig::parse_s32(ContainerOp op, const std::string &text, int32_t &value) {
	switch (op) {
	case ContainerOp::ADD:
	case ContainerOp::DEL_VALUE:
		break;

	case ContainerOp::DEL_POSITION:
	case ContainerOp::MOVE_POSITION:
	case ContainerOp::COPY_POSITION:
	default:
		return true;
	}

	char *end;
	long long ll_value = std::strtoll(text.c_str(), &end, 0);

	if (text.empty()
			|| ll_value < std::numeric_limits<typeof(value)>::min()
			|| ll_value > std::numeric_limits<typeof(value)>::max())
		return false;

	value = ll_value;
	return end[0] == '\0';
}

bool ScriptConfig::parse_rgb(ContainerOp op, const std::string &text, int32_t &value) {
	switch (op) {
	case ContainerOp::ADD:
	case ContainerOp::DEL_VALUE:
		break;

	case ContainerOp::DEL_POSITION:
	case ContainerOp::MOVE_POSITION:
	case ContainerOp::COPY_POSITION:
	default:
		return true;
	}

	char *end;
	long long ll_value = std::strtoll(text.c_str() + (text[0] == '#' ? 1 : 0), &end, 16);

	if (text.empty()
			|| ll_value < std::numeric_limits<typeof(value)>::min()
			|| ll_value > std::numeric_limits<typeof(value)>::max())
		return false;

	if (ll_value & ~0xFFFFFF)
		return false;

	value = ll_value;
	return end[0] == '\0';
}

template <class T, class V>
Result ScriptConfig::modify_container(T &container, V value, ContainerOp op, size_t index1, size_t index2) {
	switch (op) {
	case ContainerOp::ADD:
		container::add(container, std::move(value), index1);
		break;

	case ContainerOp::DEL_VALUE: {
			auto it = container::find_first(container, value);

			if (it == container.end())
				return Result::NOT_FOUND;

			container.erase(it);
			break;
		}

	case ContainerOp::DEL_POSITION:
		if (index1 < container.size()) {
			container.erase(std::next(container.begin(), index1));
		} else {
			return Result::NOT_FOUND;
		}
		break;

	case ContainerOp::MOVE_POSITION:
		if (index1 < container.size()) {
			value = *std::next(container.begin(), index1);
			if (index2 > index1 && index2 != std::numeric_limits<typeof(index2)>::max())
				index2++;

			container::add(container, std::move(value), index2);

			if (index2 <= index1)
				index1++;
			container.erase(std::next(container.begin(), index1));
		} else {
			return Result::NOT_FOUND;
		}
		break;

	case ContainerOp::COPY_POSITION:
		if (index1 < container.size()) {
			value = *std::next(container.begin(), index1);
			container::add(container, std::move(value), index2);
		} else {
			return Result::NOT_FOUND;
		}
		break;
	}

	return Result::OK;
}

Result ScriptConfig::modify(const std::string &key, const std::string &value,
		ContainerOp op, size_t index1, size_t index2) {
	auto it = properties_.find(key);

	if (it == properties_.end())
		return Result::NOT_FOUND;

	if (values_size() > MAX_VALUES_SIZE) {
		switch (op) {
		case ContainerOp::ADD:
		case ContainerOp::COPY_POSITION:
			return Result::FULL;

		case ContainerOp::DEL_VALUE:
		case ContainerOp::DEL_POSITION:
		case ContainerOp::MOVE_POSITION:
			break;
		}
	}

	auto &property = *it->second;

	switch (property.type()) {
	case Type::LIST_U16: {
			uint16_t int_value;

			if (!parse_u16(op, value, int_value))
				return Result::OUT_OF_RANGE;

			return modify_container(property.as_u16_list().values(), int_value, op, index1, index2);
		}

	case Type::LIST_S32:
	case Type::LIST_RGB: {
			int32_t int_value;

			if (property.type() == Type::LIST_RGB
					? !parse_rgb(op, value, int_value)
					: !parse_s32(op, value, int_value))
				return Result::OUT_OF_RANGE;

			return modify_container(property.as_s32_list().values(), int_value, op, index1, index2);
		}

	case Type::SET_U16: {
			uint16_t int_value;

			if (!parse_u16(op, value, int_value))
				return Result::OUT_OF_RANGE;

			return modify_container(property.as_u16_set().values(), int_value, op, index1, index2);
		}

	case Type::SET_S32:
	case Type::SET_RGB: {
			int32_t int_value;

			if (property.type() == Type::SET_RGB
					? !parse_rgb(op, value, int_value)
					: !parse_s32(op, value, int_value))
				return Result::OUT_OF_RANGE;

			return modify_container(property.as_s32_set().values(), int_value, op, index1, index2);
		}

	case Type::BOOL:
	case Type::S32:
	case Type::RGB:
	case Type::INVALID:
	default:
		return Result::OUT_OF_RANGE;
	}
}

Result ScriptConfig::set(const std::string &key, const std::string &value) {
	auto it = properties_.find(key);

	if (it == properties_.end())
		return Result::NOT_FOUND;

	auto &property = *it->second;

	switch (property.type()) {
	case Type::BOOL:
		if (value == "true" || value == "t" || value == "1") {
			property.as_bool().set_value(true);
		} else if (value == "false" || value == "f" || value == "0") {
			property.as_bool().set_value(false);
		} else if (value == "") {
			if (!Property::clear_value(property))
				properties_.erase(it);
		} else {
			return Result::OUT_OF_RANGE;
		}
		break;

	case Type::S32:
		if (value.empty()) {
			if (!Property::clear_value(property))
				properties_.erase(it);
		} else {
			int32_t int_value;

			if (parse_s32(ContainerOp::ADD, value, int_value)) {
				property.as_s32().set_value(int_value);
			} else {
				return Result::OUT_OF_RANGE;
			}
		}
		break;

	case Type::RGB:
		if (value.empty()) {
			if (!Property::clear_value(property))
				properties_.erase(it);
		} else {
			int32_t int_value;

			if (parse_rgb(ContainerOp::ADD, value, int_value)) {
				property.as_s32().set_value(int_value);
			} else {
				return Result::OUT_OF_RANGE;
			}
		}
		break;

	case Type::LIST_U16:
	case Type::LIST_S32:
	case Type::LIST_RGB:
	case Type::SET_U16:
	case Type::SET_S32:
	case Type::SET_RGB:
	case Type::INVALID:
	default:
		return Result::OUT_OF_RANGE;
	}

	return Result::OK;
}

Result ScriptConfig::unset(const std::string &key) {
	auto it = properties_.find(key);

	if (it == properties_.end())
		return Result::NOT_FOUND;

	if (!Property::clear_value(*it->second))
		properties_.erase(it);

	return Result::OK;
}

template <class T>
void ScriptConfig::print_container_summary(const T &property,
		std::vector<char> &default_str, std::vector<char> &value_str) {
	if (property.has_default())
		std::snprintf(default_str.data(), default_str.size(), "%zu value%s",
			property.defaults().size(), property.defaults().size() == 1 ? "" : "s");

	if (property.has_value())
		std::snprintf(value_str.data(), value_str.size(), "%zu value%s",
			property.values().size(), property.values().size() == 1 ? "" : "s");
}

template <class T>
void ScriptConfig::print_container_full(Shell &shell, const std::string &key,
	const T &property, const char *type, const char *fmt) {
	shell.printfln(F("Name: %s"), key.c_str());
	shell.printfln(F("Type: %s"), type);

	if (property.has_default()) {
		bool first = true;

		shell.print(F("Defaults: "));
		for (auto &value : property.defaults()) {
			if (!first)
				shell.print(F(", "));
			shell.printf(fmt, value);
			first = false;
		}
		shell.println();
	} else {
		shell.println(F("Defaults: <none>"));
	}

	if (property.has_value()) {
		bool first = true;

		shell.print(F("Values: "));
		for (auto &value : property.values()) {
			if (!first)
				shell.print(F(", "));
			shell.printf(fmt, value);
			first = false;
		}
		shell.println();
	} else {
		shell.println(F("Values: <unset>"));
	}
}

bool ScriptConfig::print(Shell &shell, const std::string *filter_key) const {
	static const char *print_header1 = " %s Type  Default     Value";
	static const char *print_header2 = " %s ----- ----------- -----------";
	static const char *print_row = "%c%-*s %-5s %11s %11s";
	size_t max_key_length = 4;
	std::vector<std::string> keys = filtered_keys(filter_key, max_key_length);
	bool print_header = !filter_key;

	if (filter_key) {
		auto it = properties_.find(*filter_key);

		if (it != properties_.end()) {
			switch (it->second->type()) {
			case Type::BOOL:
			case Type::S32:
			case Type::RGB:
				print_header = true;
				break;

			case Type::LIST_U16:
			case Type::LIST_S32:
			case Type::LIST_RGB:
			case Type::SET_U16:
			case Type::SET_S32:
			case Type::SET_RGB:
			case Type::INVALID:
				break;
			}
		} else {
			return false;
		}
	}

	if (print_header) {
		std::string keys_header1 = "Name";
		std::string keys_header2 = "----";

		if (keys_header1.length() < max_key_length)
			keys_header1.append(max_key_length - keys_header1.length(), ' ');
		while (keys_header2.length() < max_key_length)
			keys_header2.append(max_key_length - keys_header2.length(), '-');

		shell.printfln(print_header1, keys_header1.c_str());
		shell.printfln(print_header2, keys_header2.c_str());
	}

	std::sort(keys.begin(), keys.end());

	for (auto &key : keys) {
		auto &property = *properties_.at(key);
		const char *type = nullptr;
		std::vector<char> default_str(12);
		std::vector<char> value_str(12);

		switch (property.type()) {
		case Type::BOOL: {
				auto prop = property.as_bool();

				type = "bool";

				if (prop.has_default())
					std::strncpy(default_str.data(), prop.get_default() ? "true" : "false", default_str.size());

				if (prop.has_value())
					std::strncpy(value_str.data(), prop.get_value() ? "true" : "false", value_str.size());

				break;
			}

		case Type::S32:
		case Type::RGB: {
				auto prop = property.as_s32();

				type = property.type() == Type::RGB ? "rgb" : "s32";

				if (prop.has_default())
					std::snprintf(default_str.data(), default_str.size(),
						property.type() == Type::RGB ? "#%06X" : "%11d", prop.get_default());

				if (prop.has_value())
					std::snprintf(value_str.data(), value_str.size(),
						property.type() == Type::RGB ? "#%06X" : "%11d", prop.get_value());
				break;
			}

		case Type::LIST_U16:
			type = "[u16]";
			if (filter_key) {
				print_container_full(shell, key, property.as_u16_list(), type, "%u");
				continue;
			} else {
				print_container_summary(property.as_u16_list(), default_str, value_str);
			}
			break;

		case Type::LIST_S32:
		case Type::LIST_RGB:
			type = property.type() == Type::LIST_RGB ? "[rgb]": "[s32]";
			if (filter_key) {
				print_container_full(shell, key, property.as_s32_list(), type,
					property.type() == Type::LIST_RGB ? "#%06X" : "%d");
				continue;
			} else {
				print_container_summary(property.as_s32_list(), default_str, value_str);
			}
			break;

		case Type::SET_U16:
			type = "{u16}";
			if (filter_key) {
				print_container_full(shell, key, property.as_u16_set(), type, "%u");
				continue;
			} else {
				print_container_summary(property.as_u16_set(), default_str, value_str);
			}
			break;

		case Type::SET_S32:
		case Type::SET_RGB:
			type = property.type() == Type::SET_RGB ? "{rgb}": "{s32}";
			if (filter_key) {
				print_container_full(shell, key, property.as_s32_set(), type,
					property.type() == Type::SET_RGB ? "#%06X" : "%d");
				continue;
			} else {
				print_container_summary(property.as_s32_set(), default_str, value_str);
			}
			break;

		case Type::INVALID:
		default:
			continue;
		}

		if (default_str.data()[0] == '\0')
			std::strncpy(default_str.data(), "<none>", default_str.size());

		if (value_str.data()[0] == '\0')
			std::strncpy(value_str.data(), "<unset>", value_str.size());

		shell.printfln(print_row, property.registered() ? ' ' : '!', max_key_length,
			key.data(), type, default_str.data(), value_str.data());
	}

	return true;
}

bool ScriptConfig::clear() {
	bool changed = false;

	for (auto it = properties_.begin(); it != properties_.end(); ) {
		if (Property::has_value(*it->second))
			changed = true;

		if (!Property::clear_value(*it->second)) {
			it = properties_.erase(it);
		} else {
			++it;
		}
	}

	return changed;
}

template <class T>
Result ScriptConfig::load_container_uint(qindesign::cbor::Reader &reader,
		const std::string &key, Property &property, T &values, size_t total_size) {
	auto result = Result::OK;
	uint64_t entries;
	bool indefinite;

	if (!cbor::expectArray(reader, &entries, &indefinite) || indefinite) {
		if (VERBOSE)
			logger_.trace(F("Parse error reading key \"%s\" array length"), key.c_str());
		return Result::PARSE_ERROR;
	}

	while (entries-- > 0) {
		uint64_t value;

		if (!cbor::expectUnsignedInt(reader, &value)) {
			if (VERBOSE)
				logger_.trace(F("Parse error reading key \"%s\""), key.c_str());
			return Result::PARSE_ERROR;
		}

		if (result == Result::OK) {
			container::add(values, std::move(value));

			if (total_size + property.size(true) > MAX_VALUES_SIZE)
				result = Result::FULL;
		}
	}

	return result;
}

template <class T>
Result ScriptConfig::load_container_int(qindesign::cbor::Reader &reader,
		const std::string &key, Property &property, T &values, int32_t mask,
		size_t total_size) {
	auto result = Result::OK;
	uint64_t entries;
	bool indefinite;

	if (!cbor::expectArray(reader, &entries, &indefinite) || indefinite) {
		if (VERBOSE)
			logger_.trace(F("Parse error reading key \"%s\" array length"), key.c_str());
		return Result::PARSE_ERROR;
	}

	while (entries-- > 0) {
		int64_t value;

		if (!cbor::expectInt(reader, &value)) {
			if (VERBOSE)
				logger_.trace(F("Parse error reading key \"%s\""), key.c_str());
			return Result::PARSE_ERROR;
		}

		if (result == Result::OK) {
			value &= mask;
			container::add(values, std::move(value));

			if (total_size + property.size(true) > MAX_VALUES_SIZE)
				result = Result::FULL;
		}
	}

	return result;
}

Result ScriptConfig::load(cbor::Reader &reader) {
	uint64_t entries;
	bool indefinite;
	Result result = Result::OK;

	if (!cbor::expectMap(reader, &entries, &indefinite) || indefinite) {
		logger_.trace(F("Config does not contain a definite length map"));
		return Result::PARSE_ERROR;
	}

	clear();

	size_t total_size = values_size();

	while (entries-- > 0) {
		std::string key;

		if (!app::read_text(reader, key)) {
			if (VERBOSE)
				logger_.trace(F("Config map does not have a text key"));
			return Result::PARSE_ERROR;
		}

		auto pos = key.find_first_of('/');
		if (pos == std::string::npos) {
			if (VERBOSE)
				logger_.trace(F("Invalid key/type \"%s\""), key.c_str());
			return Result::PARSE_ERROR;
		}

		Type type = type_of(key.substr(pos + 1));
		if (type == Type::INVALID) {
			if (VERBOSE)
				logger_.trace(F("Invalid key/type \"%s\""), key.c_str());
			return Result::PARSE_ERROR;
		}

		key = key.substr(0, pos);
		if (!allowed_key(key)) {
			if (VERBOSE)
				logger_.trace(F("Invalid key \"%s\""), key.c_str());
			return Result::PARSE_ERROR;
		}

		if (result != Result::OK) {
			// Skip value (full)
			if (!reader.isWellFormed()) {
				if (VERBOSE)
					logger_.trace(F("Value for key \"%s\" is not well-formed"), key.c_str());
				return Result::PARSE_ERROR;
			} else if (VERBOSE) {
				logger_.trace(F("Skip value for key \"%s\" (full)"), key.c_str());
			}
			continue;
		}

		auto emplace = properties_.emplace(key, std::move(ScriptConfig::Property::create(type, false)));
		if (!emplace.second) {
			if (emplace.first->second->type() != type) {
				// Skip value (type mismatch)
				if (!reader.isWellFormed()) {
					if (VERBOSE)
						logger_.trace(F("Value for key \"%s\" is not well-formed"), key.c_str());
					return Result::PARSE_ERROR;
				} else if (VERBOSE) {
					logger_.trace(F("Skip value for key \"%s\" (type mismatch)"), key.c_str());
				}
				continue;
			}

			/* Existing properties have already had the value cleared */
		} else {
			total_size += entry_base_size(key);
		}

		auto &property = *emplace.first->second;

		if (total_size + property.size(true) > MAX_VALUES_SIZE) {
			result = Result::FULL;
			if (emplace.second)
				properties_.erase(emplace.first);

			// Skip value (full)
			if (!reader.isWellFormed()) {
				if (VERBOSE)
					logger_.trace(F("Value for key \"%s\" is not well-formed"), key.c_str());
				return Result::PARSE_ERROR;
			} else if (VERBOSE) {
				logger_.trace(F("Skip value for key \"%s\" (full)"), key.c_str());
			}
			continue;
		}

		switch (type) {
		case Type::BOOL: {
				bool value;

				if (!cbor::expectBoolean(reader, &value)) {
					if (VERBOSE)
						logger_.trace(F("Parse error reading key \"%s\""), key.c_str());
					return Result::PARSE_ERROR;
				}

				property.as_bool().set_value(value);
				break;
			}

		case Type::S32:
		case Type::RGB: {
				int64_t value;

				if (!cbor::expectInt(reader, &value)) {
					if (VERBOSE)
						logger_.trace(F("Parse error reading key \"%s\""), key.c_str());
					return Result::PARSE_ERROR;
				}

				if (type == Type::RGB)
					value &= 0xFFFFFF;

				property.as_s32().set_value(value);
				break;
			}

		case Type::LIST_U16:
			if (downgrade_result(result,
					load_container_uint(reader, key, property,
						property.as_u16_list().values(), total_size))
					>= Result::PARSE_ERROR) {
				return result;
			}
			break;

		case Type::LIST_S32:
		case Type::LIST_RGB:
			if (downgrade_result(result,
					load_container_int(reader, key, property,
						property.as_s32_list().values(),
						type == Type::LIST_RGB ? 0xFFFFFF : ~0, total_size))
					>= Result::PARSE_ERROR) {
				return result;
			}
			break;

		case Type::SET_U16:
			if (downgrade_result(result,
					load_container_uint(reader, key, property,
						property.as_u16_set().values(), total_size))
					>= Result::PARSE_ERROR) {
				return result;
			}
			break;

		case Type::SET_S32:
		case Type::SET_RGB:
			if (downgrade_result(result,
					load_container_int(reader, key, property,
						property.as_s32_set().values(),
						type == Type::LIST_RGB ? 0xFFFFFF : ~0, total_size))
					>= Result::PARSE_ERROR) {
				return result;
			}
			break;

		case Type::INVALID:
		default:
			return Result::PARSE_ERROR;
		}

		if (result > Result::FULL)
			return result;

		total_size += property.size(true);
	}

	return result;
}

void ScriptConfig::write_key(cbor::Writer &writer, const std::string &key, const char *type) {
	size_t type_length = strlen(type);
	writer.beginText(key.length() + 1 + type_length);
	writer.writeBytes(reinterpret_cast<const uint8_t*>(key.c_str()), key.length());
	writer.write('/');
	writer.writeBytes(reinterpret_cast<const uint8_t*>(type), type_length);
}

template <class T>
void ScriptConfig::save_container_uint(cbor::Writer &writer,
		const std::string &key, const char *type, T &values) {
	if (!values.empty()) {
		write_key(writer, key, type);
		writer.beginArray(values.size());
		for (auto &value : values) {
			writer.writeUnsignedInt(value);
		}
	}
}

template <class T>
void ScriptConfig::save_container_int(cbor::Writer &writer,
		const std::string &key, const char *type, T &values) {
	if (!values.empty()) {
		write_key(writer, key, type);
		writer.beginArray(values.size());
		for (auto &value : values) {
			writer.writeInt(value);
		}
	}
}

void ScriptConfig::save(cbor::Writer &writer) {
	size_t count = 0;

	for (const auto &entry : properties_) {
		auto &property = *entry.second;

		switch (property.type()) {
		case Type::BOOL:
			if (property.as_bool().has_value())
				count++;
			break;

		case Type::S32:
		case Type::RGB:
			if (property.as_s32().has_value())
				count++;
			break;

		case Type::LIST_U16:
			if (property.as_u16_list().has_value())
				count++;
			break;

		case Type::LIST_S32:
		case Type::LIST_RGB:
			if (property.as_s32_list().has_value())
				count++;
			break;

		case Type::SET_U16:
			if (property.as_u16_set().has_value())
				count++;
			break;

		case Type::SET_S32:
		case Type::SET_RGB:
			if (property.as_s32_set().has_value())
				count++;
			break;

		case Type::INVALID:
		default:
			break;
		}
	}

	writer.beginMap(count);

	for (const auto &entry : properties_) {
		auto &key = entry.first;
		auto &property = *entry.second;

		switch (property.type()) {
		case Type::BOOL:
			if (property.as_bool().has_value()) {
				write_key(writer, key, "bool");
				writer.writeBoolean(property.as_bool().get_value());
			}
			break;

		case Type::S32:
		case Type::RGB:
			if (property.as_s32().has_value()) {
				write_key(writer, key, property.type() == Type::RGB ? "rgb": "s32");
				writer.writeInt(property.as_s32().get_value());
			}
			break;

		case Type::LIST_U16:
			save_container_uint(writer, key, "list_u16", property.as_u16_list().values());
			break;

		case Type::LIST_S32:
		case Type::LIST_RGB:
			save_container_int(writer, key,
				property.type() == Type::LIST_RGB ? "list_rgb" : "list_s32",
				property.as_s32_list().values());
			break;

		case Type::SET_U16:
			save_container_uint(writer, key, "set_u16", property.as_u16_set().values());
			break;

		case Type::SET_S32:
		case Type::SET_RGB:
			save_container_int(writer, key,
				property.type() == Type::SET_RGB ? "set_rgb" : "set_s32",
				property.as_s32_set().values());
			break;

		case Type::INVALID:
		default:
			break;
		}
	}
}

} // namespace aurcor
