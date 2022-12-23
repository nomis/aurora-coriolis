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

#include <uuid/log.h>

#include "aurcor/micropython.h"
#include "aurcor/util.h"

namespace cbor = qindesign::cbor;

namespace aurcor {

inline ScriptConfig::Property::pointer_type ScriptConfig::Property::create(ScriptConfig::Type type) {
	switch (type) {
	case BOOL:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::BoolProperty()};

	case INT32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::Int32Property(false)};

	case RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::Int32Property(true)};

	case LIST_UINT16:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListUInt16Property()};

	case LIST_INT32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListInt32Property(false)};

	case LIST_RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::ListInt32Property(true)};

	case SET_UINT16:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetUInt16Property()};

	case SET_INT32:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetInt32Property(false)};

	case SET_RGB:
		return ScriptConfig::Property::pointer_type{new ScriptConfig::SetInt32Property(true)};

	case INVALID:
	default:
		return {nullptr};
	}
}

void ScriptConfig::Property::Deleter::operator()(Property *property) {
	switch (property->type()) {
	case BOOL:
		delete static_cast<ScriptConfig::BoolProperty*>(property);
		break;

	case INT32:
		delete static_cast<ScriptConfig::Int32Property*>(property);
		break;

	case RGB:
		delete static_cast<ScriptConfig::Int32Property*>(property);
		break;

	case LIST_UINT16:
		delete static_cast<ScriptConfig::ListUInt16Property*>(property);
		break;

	case LIST_INT32:
		delete static_cast<ScriptConfig::ListInt32Property*>(property);
		break;

	case LIST_RGB:
		delete static_cast<ScriptConfig::ListInt32Property*>(property);
		break;

	case SET_UINT16:
		delete static_cast<ScriptConfig::SetUInt16Property*>(property);
		break;

	case SET_INT32:
		delete static_cast<ScriptConfig::SetInt32Property*>(property);
		break;

	case SET_RGB:
		delete static_cast<ScriptConfig::SetInt32Property*>(property);
		break;

	case INVALID:
	default:
		delete property;
		break;
	}
}

size_t ScriptConfig::Property::size(bool values) const {
	switch (type()) {
	case BOOL:
		return as_bool().size(values);

	case INT32:
	case RGB:
		return as_int32().size(values);

	case LIST_UINT16:
		return as_int32_list().size(values);

	case LIST_INT32:
	case LIST_RGB:
		return as_int32_list().size(values);

	case SET_UINT16:
		return as_uint16_set().size(values);

	case SET_INT32:
	case SET_RGB:
		return as_int32_set().size(values);

	case INVALID:
	default:
		return 0;
	}
}

bool ScriptConfig::Property::clear_default(Property &property) {
	switch (property.type()) {
	case BOOL:
		property.as_bool().clear_default();
		return property.as_bool().has_value();

	case INT32:
	case RGB:
		property.as_int32().clear_default();
		return property.as_int32().has_value();

	case LIST_UINT16:
		property.as_int32_list().clear_default();
		return property.as_int32_list().has_value();

	case LIST_INT32:
	case LIST_RGB:
		property.as_int32_list().clear_default();
		return property.as_int32_list().has_value();

	case SET_UINT16:
		property.as_uint16_set().clear_default();
		return property.as_uint16_set().has_value();

	case SET_INT32:
	case SET_RGB:
		property.as_int32_set().clear_default();
		return property.as_int32_set().has_value();

	case INVALID:
	default:
		return false;
	}
}

bool ScriptConfig::Property::clear_value(Property &property) {
	switch (property.type()) {
	case BOOL:
		property.as_bool().clear_value();
		return property.as_bool().has_default();

	case INT32:
	case RGB:
		property.as_int32().clear_value();
		return property.as_int32().has_default();

	case LIST_UINT16:
		property.as_int32_list().clear_value();
		return property.as_int32_list().has_default();

	case LIST_INT32:
	case LIST_RGB:
		property.as_int32_list().clear_value();
		return property.as_int32_list().has_default();

	case SET_UINT16:
		property.as_uint16_set().clear_value();
		return property.as_uint16_set().has_default();

	case SET_INT32:
	case SET_RGB:
		property.as_int32_set().clear_value();
		return property.as_int32_set().has_default();

	case INVALID:
	default:
		return false;
	}
}

ScriptConfig::Type ScriptConfig::type_of(const std::string &type) {
	if (type == "o") {
		return Type::BOOL;
	} else if (type == "i") {
		return Type::INT32;
	} else if (type == "C") {
		return Type::RGB;
	} else if (type == "[H") {
		return Type::LIST_UINT16;
	} else if (type == "[i") {
		return Type::LIST_INT32;
	} else if (type == "[C") {
		return Type::LIST_RGB;
	} else if (type == "{H") {
		return Type::SET_UINT16;
	} else if (type == "{i") {
		return Type::SET_INT32;
	} else if (type == "{C") {
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

void ScriptConfig::register_config(mp_obj_t dict) {
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

		if (elem == NULL) {
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
			total_size -= property_it->second->size(false);
		} else {
			property_it = properties_.emplace(key, std::move(ScriptConfig::Property::create(type))).first;
			total_size += entry_base_size(key);
		}

		ScriptConfig::Property &property = *property_it->second;
		auto *mp_obj_get_int_not_const = reinterpret_cast<mp_int_t (*)(mp_obj_t)>(&mp_obj_get_int);

		/* Existing properties have already had the default cleared */

		if (total_size + property.size(false) > MAX_DEFAULTS_SIZE)
			mp_raise_ValueError(MP_ERROR_TEXT("maximum config size exceeded"));

		if (default_value != mp_const_none) {
			switch (type) {
			case BOOL:
				property.as_bool().set_default(mp_obj_is_true(default_value));
				break;

			case INT32:
				property.as_int32().set_default(mp_obj_get_int(default_value));
				break;

			case RGB:
				property.as_int32().set_default(convert_rgb_value(default_value));
				break;

			case LIST_UINT16:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_uint16_list(), total_size);
				break;

			case LIST_INT32:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_int32_list(), total_size);
				break;

			case LIST_RGB:
				convert_container_values(default_value, convert_rgb_value, property.as_int32_list(), total_size);
				break;

			case SET_UINT16:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_uint16_set(), total_size);
				break;

			case SET_INT32:
				convert_container_values(default_value, mp_obj_get_int_not_const, property.as_int32_set(), total_size);
				break;

			case SET_RGB:
				convert_container_values(default_value, convert_rgb_value, property.as_int32_set(), total_size);
				break;

			case INVALID:
				break;
			}
		}

		total_size += property.size(false);
	}

	micropython_nlr_finally();
	micropython_nlr_end();
}

void ScriptConfig::populate_config(mp_obj_t dict) {
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

		switch (property.type()) {
		case BOOL:
			if (property.as_bool().has_any()) {
				elem->value = mp_obj_new_bool(property.as_bool().get_any());
			} else {
				elem->value = mp_const_none;
			}
			break;

		case INT32:
		case RGB:
			if (property.as_int32().has_any()) {
				elem->value = MP_OBJ_NEW_SMALL_INT(property.as_int32().get_any());
			} else {
				elem->value = mp_const_none;
			}
			break;

		case LIST_UINT16:
			elem->value = create_list(property.as_uint16_list().get_any());
			break;

		case LIST_INT32:
		case LIST_RGB:
			elem->value = create_list(property.as_int32_list().get_any());
			break;

		case SET_UINT16:
			elem->value = create_set(property.as_uint16_set().get_any(), set_uint16_it);
			break;

		case SET_INT32:
		case SET_RGB:
			elem->value = create_set(property.as_int32_set().get_any(), set_int32_it);
			break;

		case INVALID:
			break;
		}
	}

	micropython_nlr_finally();
	micropython_nlr_end();
}

} // namespace aurcor
