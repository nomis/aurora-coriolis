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

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

extern "C" {
	#include <py/obj.h>
}

#include <CBOR.h>
#include <CBOR_parsing.h>
#include <CBOR_streams.h>

#include <uuid/log.h>

#include "aurcor/util.h"

namespace aurcor {

class ScriptConfig {
public:
	static constexpr size_t MAX_KEY_LENGTH = 48;
	static constexpr size_t MAX_DEFAULTS_SIZE = 1024 * sizeof(uintptr_t);
	static constexpr size_t MAX_VALUES_SIZE = 1024 * sizeof(uintptr_t);

	enum Type {
		BOOL,
		INT32,
		RGB,

		LIST_UINT16,
		LIST_INT32,
		LIST_RGB,

		SET_UINT16,
		SET_INT32,
		SET_RGB,

		INVALID,
	};

	class BoolProperty;
	class Int32Property;
	class ListUInt16Property;
	class ListInt32Property;
	class SetUInt16Property;
	class SetInt32Property;

	class Property {
	private:
		class Deleter {
		public:
			void operator()(Property *property);
		};

	public:
		using pointer_type = std::unique_ptr<Property, Deleter>;

		static pointer_type create(Type type);

		/** Clear default and @return has_value() */
		static bool clear_default(Property &property);
		/** Clear value and @return has_default() */
		static bool clear_value(Property &property);

		inline Type type() const { return type_; }
		inline void type(Type type) { type_ = type; }

		size_t size(bool values) const;

		inline BoolProperty& as_bool() { return static_cast<BoolProperty&>(*this); }
		inline Int32Property& as_int32() { return static_cast<Int32Property&>(*this); }
		inline ListUInt16Property& as_uint16_list() { return static_cast<ListUInt16Property&>(*this); }
		inline ListInt32Property& as_int32_list() { return static_cast<ListInt32Property&>(*this); }
		inline SetUInt16Property& as_uint16_set() { return static_cast<SetUInt16Property&>(*this); }
		inline SetInt32Property& as_int32_set() { return static_cast<SetInt32Property&>(*this); }

		inline const BoolProperty& as_bool() const { return static_cast<const BoolProperty&>(*this); }
		inline const Int32Property& as_int32() const { return static_cast<const Int32Property&>(*this); }
		inline const ListUInt16Property& as_uint16_list() const { return static_cast<const ListUInt16Property&>(*this); }
		inline const ListInt32Property& as_int32_list() const { return static_cast<const ListInt32Property&>(*this); }
		inline const SetUInt16Property& as_uint16_set() const { return static_cast<const SetUInt16Property&>(*this); }
		inline const SetInt32Property& as_int32_set() const { return static_cast<const SetInt32Property&>(*this); }

	protected:
		Property(Type type) { type_ = type; has_default_ = false; has_value_ = false; }

		inline bool has_default() const { return has_default_; }
		inline void default_set() { has_default_ = true; }
		inline void clear_default() { has_default_ = false; }

		inline bool has_value() const { return has_value_; }
		inline void value_set() { has_value_ = true; }
		inline void clear_value() { has_value_ = false; }

		inline bool has_any() const { return has_default() || has_value(); }

	private:
		Type type_ : 6;
		bool has_default_ : 1;
		bool has_value_ : 1;
	};

	static_assert(sizeof(Property) <= sizeof(uintptr_t), "Property is minimum size");
	static_assert(sizeof(Property::pointer_type) <= sizeof(uintptr_t), "Property pointer is minimum size");

	class BoolProperty: public Property {
	public:
		BoolProperty() : Property(Type::BOOL) {}

		inline bool get_default() const { return default_; }
		inline void set_default(bool value) { default_ = default_; default_set(); }

		inline bool get_value() const { return value_; }
		inline void set_value(bool value) { value_ = value; value_set(); }

		inline bool get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<BoolProperty>(); }

	private:
		bool default_ : 1;
		bool value_ : 1;
	};

	static_assert(sizeof(BoolProperty) <= sizeof(uintptr_t), "BoolProperty is minimum size");

	class Int32Property: public Property {
	public:
		Int32Property(bool rgb) : Property(rgb ? Type::RGB : Type::INT32) {}

		inline int32_t get_default() const { return default_; }
		inline void set_default(int32_t value) { default_ = value; default_set(); }

		inline int32_t get_value() const { return value_; }
		inline void set_value(int32_t value) { value_ = value; value_set(); }

		inline int32_t get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<Int32Property>(); }

	private:
		int32_t default_;
		int32_t value_;
	};

	static_assert(sizeof(Int32Property) <= 3 * sizeof(uintptr_t), "Int32Property is minimum size");

	class ListUInt16Property: public Property {
	public:
		ListUInt16Property() : Property(Type::LIST_UINT16) {}

		inline std::vector<uint16_t>& defaults() { return defaults_; }
		inline const std::vector<uint16_t>& defaults() const { return defaults_; }
		inline bool has_default() const { return !defaults_.empty(); }
		inline void clear_default() { defaults_.clear(); }

		inline std::vector<uint16_t>& values() { return values_; }
		inline const std::vector<uint16_t>& values() const { return values_; }
		inline bool has_value() const { return !values_.empty(); }
		inline void clear_value() { values_.clear(); }

		inline const std::vector<uint16_t>& get_any() const { if (has_value()) { return values(); } else { return defaults(); } }

		size_t size(bool values) const {
			return rounded_sizeof<ListUInt16Property>()
				+ rounded_sizeof<uint16_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::vector<uint16_t> defaults_;
		std::vector<uint16_t> values_;
	};

	class ListInt32Property: public Property {
	public:
		ListInt32Property(bool rgb) : Property(rgb ? Type::LIST_RGB : Type::LIST_INT32) {}

		inline std::vector<int32_t>& defaults() { return defaults_; }
		inline const std::vector<int32_t>& defaults() const { return defaults_; }
		inline bool has_default() const { return !defaults_.empty(); }
		inline void clear_default() { defaults_.clear(); }

		inline std::vector<int32_t>& values() { return values_; }
		inline const std::vector<int32_t>& values() const { return values_; }
		inline bool has_value() const { return !values_.empty(); }
		inline void clear_value() { values_.clear(); }

		inline const std::vector<int32_t>& get_any() const { if (has_value()) { return values(); } else { return defaults(); } }

		size_t size(bool values) const {
			return rounded_sizeof<ListInt32Property>()
				+ rounded_sizeof<int32_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::vector<int32_t> defaults_;
		std::vector<int32_t> values_;
	};

	class SetUInt16Property: public Property {
	public:
		SetUInt16Property() : Property(Type::SET_UINT16) {}

		inline std::set<uint16_t>& defaults() { return defaults_; }
		inline const std::set<uint16_t>& defaults() const { return defaults_; }
		inline bool has_default() const { return !defaults_.empty(); }
		inline void clear_default() { defaults_.clear(); }

		inline std::set<uint16_t>& values() { return values_; }
		inline const std::set<uint16_t>& values() const { return values_; }
		inline bool has_value() const { return !values_.empty(); }
		inline void clear_value() { values_.clear(); }

		inline const std::set<uint16_t>& get_any() const { if (has_value()) { return values(); } else { return defaults(); } }

		size_t size(bool values) const {
			return rounded_sizeof<SetUInt16Property>()
				+ rounded_sizeof<uint16_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::set<uint16_t> defaults_;
		std::set<uint16_t> values_;
	};

	class SetInt32Property: public Property {
	public:
		SetInt32Property(bool rgb) : Property(rgb ? Type::SET_RGB : Type::SET_INT32) {}

		inline std::set<int32_t>& defaults() { return defaults_; }
		inline const std::set<int32_t>& defaults() const { return defaults_; }
		inline bool has_default() const { return !defaults_.empty(); }
		inline void clear_default() { defaults_.clear(); }

		inline std::set<int32_t>& values() { return values_; }
		inline const std::set<int32_t>& values() const { return values_; }
		inline bool has_value() const { return !values_.empty(); }
		inline void clear_value() { values_.clear(); }

		inline const std::set<int32_t>& get_any() const { if (has_value()) { return values(); } else { return defaults(); } }

		size_t size(bool values) const {
			return rounded_sizeof<SetInt32Property>()
				+ rounded_sizeof<int32_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::set<int32_t> defaults_;
		std::set<int32_t> values_;
	};

	static Type type_of(const std::string &type);

	ScriptConfig() = default;
	~ScriptConfig() = default;

	void register_config(mp_obj_t dict);
	void populate_config(mp_obj_t dict);

	bool load(qindesign::cbor::Reader &reader);
	void save(qindesign::cbor::Writer &writer);

private:
	static size_t entry_base_size(const std::string &key);
	static size_t entry_size(const std::string &key, const Property &value, bool values);
	static bool allowed_key(const std::string &key);
	static mp_int_t convert_rgb_value(mp_obj_t value_obj);
	template <class T>
	static void convert_container_values(mp_obj_t container_obj, mp_int_t (*convert_value)(mp_obj_t value_obj), T &property, size_t total_size);
	template <class T>
	static mp_obj_t create_list(const std::vector<T> &container);
	template <class T>
	static mp_obj_t create_set(const std::set<T> &container, typename std::set<T>::const_iterator &container_it);

	size_t size(bool vaules) const;
	inline size_t defaults_size() const { return size(false); }
	inline size_t values_size() const { return size(true); }

	std::unordered_map<std::string,Property::pointer_type> properties_;
};

} // namespace aurcor
