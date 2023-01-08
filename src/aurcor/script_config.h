/*
 * aurora-coriolis - ESP32 WS281x multi-channel LED controller with MicroPython
 * Copyright 2022-2023  Simon Arlott
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

#include <uuid/console.h>
#include <uuid/log.h>

#include "led_profiles.h"
#include "util.h"

#ifndef PSTR_ALIGN
# define PSTR_ALIGN 4
#endif

namespace aurcor {

class ScriptConfig {
public:
	static constexpr size_t MAX_KEY_LENGTH = 48;
	static constexpr size_t MAX_DEFAULTS_SIZE = 1024 * sizeof(uintptr_t);
	static constexpr size_t MAX_VALUES_SIZE = 1024 * sizeof(uintptr_t);

	enum Type {
		BOOL,
		S32,
		RGB,
		FLOAT,
		PROFILE,

		LIST_U16,
		LIST_S32,
		LIST_RGB,

		SET_U16,
		SET_S32,
		SET_RGB,

		INVALID,
	};

	using types_bitset = std::bitset<Type::INVALID>;

	class BoolProperty;
	class S32Property;
	class FloatProperty;
	class ProfileProperty;
	class ListU16Property;
	class ListS32Property;
	class SetU16Property;
	class SetS32Property;

	class Property {
	private:
		class Deleter {
		public:
			void operator()(Property *property);
		};

	public:
		using pointer_type = std::unique_ptr<Property, Deleter>;

		static pointer_type create(Type type, bool registered);

		static bool has_value(Property &property);
		/** Clear default and @return has_value() */
		static bool clear_default(Property &property);
		/** Clear value and @return registered() */
		static bool clear_value(Property &property);

		inline Type type() const { return type_; }
		inline void type(Type type) { type_ = type; }

		inline bool registered() const { return registered_; }
		inline void registered(bool registered) { registered_ = registered; }

		size_t size(bool values) const;

		inline BoolProperty& as_bool() { return static_cast<BoolProperty&>(*this); }
		inline S32Property& as_s32() { return static_cast<S32Property&>(*this); }
		inline FloatProperty& as_float() { return static_cast<FloatProperty&>(*this); }
		inline ProfileProperty& as_profile() { return static_cast<ProfileProperty&>(*this); }
		inline ListU16Property& as_u16_list() { return static_cast<ListU16Property&>(*this); }
		inline ListS32Property& as_s32_list() { return static_cast<ListS32Property&>(*this); }
		inline SetU16Property& as_u16_set() { return static_cast<SetU16Property&>(*this); }
		inline SetS32Property& as_s32_set() { return static_cast<SetS32Property&>(*this); }

		inline const BoolProperty& as_bool() const { return static_cast<const BoolProperty&>(*this); }
		inline const S32Property& as_s32() const { return static_cast<const S32Property&>(*this); }
		inline const FloatProperty& as_float() const { return static_cast<const FloatProperty&>(*this); }
		inline const ProfileProperty& as_profile() const { return static_cast<const ProfileProperty&>(*this); }
		inline const ListU16Property& as_u16_list() const { return static_cast<const ListU16Property&>(*this); }
		inline const ListS32Property& as_s32_list() const { return static_cast<const ListS32Property&>(*this); }
		inline const SetU16Property& as_u16_set() const { return static_cast<const SetU16Property&>(*this); }
		inline const SetS32Property& as_s32_set() const { return static_cast<const SetS32Property&>(*this); }

	protected:
		Property(Type type, bool registered) {
			type_ = type;
			registered_ = registered;
			has_default_ = false;
			has_value_ = false;
		}

		inline bool has_default() const { return has_default_; }
		inline void default_set() { has_default_ = true; }
		inline void clear_default() { has_default_ = false; }

		inline bool has_value() const { return has_value_; }
		inline void value_set() { has_value_ = true; }
		inline void clear_value() { has_value_ = false; }

		inline bool has_any() const { return has_default() || has_value(); }

	private:
		Type type_ : 5;
		bool registered_ : 1;
		bool has_default_ : 1;
		bool has_value_ : 1;
	};

	static_assert(sizeof(Property) <= sizeof(uintptr_t), "Property is minimum size");
	static_assert(sizeof(Property::pointer_type) <= sizeof(uintptr_t), "Property pointer is minimum size");

	class BoolProperty: public Property {
	public:
		BoolProperty(bool registered) : Property(Type::BOOL, registered) {}

		inline bool get_default() const { return default_; }
		inline void set_default(bool value) { default_ = value; default_set(); }

		inline bool get_value() const { return value_; }
		inline void set_value(bool value) { value_ = value; value_set(); }

		inline bool get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_value;
		using Property::clear_value;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<BoolProperty>(); }

	private:
		bool default_ : 1;
		bool value_ : 1;
	};

	static_assert(sizeof(BoolProperty) <= sizeof(uintptr_t), "BoolProperty is minimum size");

	class S32Property: public Property {
	public:
		S32Property(bool rgb, bool registered) : Property(rgb ? Type::RGB : Type::S32, registered) {}

		inline int32_t get_default() const { return default_; }
		inline void set_default(int32_t value) { default_ = value; default_set(); }

		inline int32_t get_value() const { return value_; }
		inline void set_value(int32_t value) { value_ = value; value_set(); }

		inline int32_t get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_value;
		using Property::clear_value;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<S32Property>(); }

	private:
		int32_t default_;
		int32_t value_;
	};

	static_assert(sizeof(S32Property) <= 3 * sizeof(uintptr_t), "S32Property is minimum size");

	class FloatProperty: public Property {
	public:
		FloatProperty(bool registered) : Property(Type::FLOAT, registered) {}

		inline float get_default() const { return default_; }
		inline void set_default(float value) { default_ = value; default_set(); }

		inline float get_value() const { return value_; }
		inline void set_value(float value) { value_ = value; value_set(); }

		inline float get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_value;
		using Property::clear_value;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<FloatProperty>(); }

	private:
		float default_;
		float value_;
	};

	static_assert(sizeof(FloatProperty) <= 3 * sizeof(uintptr_t), "FloatProperty is minimum size");

	class ProfileProperty: public Property {
	public:
		ProfileProperty(bool registered) : Property(Type::PROFILE, registered) {}

		inline enum led_profile_id get_default() const { return default_; }
		inline void set_default(enum led_profile_id value) { default_ = value; default_set(); }

		inline enum led_profile_id get_value() const { return value_; }
		inline void set_value(enum led_profile_id value) { value_ = value; value_set(); }

		inline enum led_profile_id get_any() const { if (has_value()) { return get_value(); } else { return get_default(); } }

		using Property::has_default;
		using Property::clear_default;
		using Property::has_value;
		using Property::clear_value;
		using Property::has_any;

		size_t size(bool values) const { return rounded_sizeof<ProfileProperty>(); }

	private:
		enum led_profile_id default_;
		enum led_profile_id value_;
	};

	static_assert(sizeof(ProfileProperty) <= 3 * sizeof(uintptr_t), "ProfileProperty is minimum size");

	class ListU16Property: public Property {
	public:
		ListU16Property(bool registered) : Property(Type::LIST_U16, registered) {}

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
			return rounded_sizeof<ListU16Property>()
				+ rounded_sizeof<uint16_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::vector<uint16_t> defaults_;
		std::vector<uint16_t> values_;
	};

	class ListS32Property: public Property {
	public:
		ListS32Property(bool rgb, bool registered) : Property(rgb ? Type::LIST_RGB : Type::LIST_S32, registered) {}

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
			return rounded_sizeof<ListS32Property>()
				+ rounded_sizeof<int32_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::vector<int32_t> defaults_;
		std::vector<int32_t> values_;
	};

	class SetU16Property: public Property {
	public:
		SetU16Property(bool registered) : Property(Type::SET_U16, registered) {}

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
			return rounded_sizeof<SetU16Property>()
				+ rounded_sizeof<uint16_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::set<uint16_t> defaults_;
		std::set<uint16_t> values_;
	};

	class SetS32Property: public Property {
	public:
		SetS32Property(bool rgb, bool registered) : Property(rgb ? Type::SET_RGB : Type::SET_S32, registered) {}

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
			return rounded_sizeof<SetS32Property>()
				+ rounded_sizeof<int32_t>() * (values ? values_ : defaults_).size();
		}

	private:
		std::set<int32_t> defaults_;
		std::set<int32_t> values_;
	};

	enum class ContainerOp {
		ADD,
		DEL_VALUE,
		DEL_POSITION,
		MOVE_POSITION,
		COPY_POSITION,
		SET_POSITION,
	};

	static Type type_of(const std::string &type);

	ScriptConfig() = default;
	~ScriptConfig() = default;

	void register_properties(mp_obj_t dict);
	void populate_dict(mp_obj_t dict);

	std::vector<std::string> keys(types_bitset types) const;
	Type key_type(const std::string &key) const;
	std::vector<std::string> container_values(const std::string &key) const;
	Result modify(const std::string &key, const std::string &value, ContainerOp op, size_t index1 = 0, size_t index2 = 0);
	Result set(const std::string &key, const std::string &value);
	Result unset(const std::string &key);
	bool print(uuid::console::Shell &shell, const std::string *filter_key) const;
	bool clear();
	bool cleanup();

	size_t keys_size() const;
	inline size_t defaults_size() const { return size(false); }
	inline size_t values_size() const { return size(true); }

	Result load(qindesign::cbor::Reader &reader);
	void save(qindesign::cbor::Writer &writer);

private:
	static size_t entry_key_size(const std::string &key);
	static size_t entry_base_size(const std::string &key);
	static size_t entry_size(const std::string &key, const Property &value, bool values);
	static bool allowed_key(const std::string &key);

	template <class T>
	static void print_container_summary(const T &property,
			std::vector<char> &default_str, std::vector<char> &value_str);
	template <class T>
	static void print_container_full(uuid::console::Shell &shell, const std::string &key,
		const T &property, const char *type, const char *fmt);

	template <class T>
	static void container_values(const T &property, const char *fmt, std::vector<std::string> &values);

	static bool parse_u16(ContainerOp op, const std::string &text, uint16_t &value);
	static bool parse_s32(ContainerOp op, const std::string &text, int32_t &value);
	static bool parse_rgb(ContainerOp op, const std::string &text, int32_t &value);
	static bool parse_float(ContainerOp op, const std::string &text, float &value);
	static bool parse_profile(ContainerOp op, const std::string &text, enum led_profile_id &value);

	template <class T, class V>
	static Result modify_container(T &container, V value, ContainerOp op, size_t index1, size_t index2);

	static mp_int_t convert_rgb_value(mp_obj_t value_obj);
	template <class T>
	static void convert_container_values(mp_obj_t container_obj, mp_int_t (*convert_value)(mp_obj_t value_obj), T &property, size_t total_size);
	template <class T>
	static mp_obj_t create_list(const std::vector<T> &container);
	template <class T>
	static mp_obj_t create_set(const std::set<T> &container, typename std::set<T>::const_iterator &container_it);

	static void write_key(qindesign::cbor::Writer &writer, const std::string &key, const char *type);
	template <class T>
	static Result load_container_uint(qindesign::cbor::Reader &reader,
		const std::string &key, Property &property, T &values, size_t total_size);
	template <class T>
	static Result load_container_int(qindesign::cbor::Reader &reader,
		const std::string &key, Property &property, T &values, int32_t mask, size_t total_size);
	template <class T>
	static void save_container_uint(qindesign::cbor::Writer &writer,
		const std::string &key, const char *type, T &values);
	template <class T>
	static void save_container_int(qindesign::cbor::Writer &writer,
		const std::string &key, const char *type, T &values);

	static uuid::log::Logger logger_;
#ifdef ENV_NATIVE
	static constexpr bool VERBOSE = true;
#else
	static constexpr bool VERBOSE = false;
#endif

	std::vector<std::string> filtered_keys(const std::string *filter_key, size_t &max_key_length) const;

	size_t size(bool vaules) const;

	std::unordered_map<std::string,Property::pointer_type> properties_;
};

} // namespace aurcor
