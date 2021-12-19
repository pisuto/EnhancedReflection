#pragma once

#include <vector>
#include <iostream>
#include <map>
#include <cstddef>

#include "ref_base.h"
#include "../tool/ref_helper.h"

namespace ref {

	/* 解析类的父类 */
	template<typename T>
	struct type_struct_base : type_descriptor {
		using type_class    = typename T;
		using type_pointer  = typename T*;
		using init_func_ptr = void(*)(type_struct_base*);

		type_struct_base(init_func_ptr init) : type_descriptor(nullptr, 0, type_is_customize_v<T>) {
			init(this);
		}
		type_struct_base(const char* name, size_t size, const std::initializer_list<type_member>& init) : type_descriptor(nullptr, 0, type_is_customize_v<T>),
			members(init) {}

		virtual void serialize(file_parser* parser, const void* obj, int level) override {
			parser->write_customize_type(this, obj, level);
		}

		virtual void deserialize(file_parser* parser, std::string var, const void* obj, int level) override {
			for (auto& element : members)
			{
				element.type_desc->deserialize(parser, element.var_name, (char*)obj + element.offset , level + 1);
			}
		}

		virtual void type_members(std::vector<type_member>& temp_members) { temp_members = members; }

		std::vector<type_member> members;
	};

	/* 解析类的派生类，用于描述vector */
	struct type_struct_vector : type_descriptor {
		using get_size_func_ptr = size_t(*)(const void*);
		using get_item_func_ptr = const void* (*)(const void*, size_t);
		using resize_func_ptr   = size_t(*)(const void*, size_t);
		get_size_func_ptr get_size;
		get_item_func_ptr get_item;
		resize_func_ptr   resize;
		type_descriptor* item_type;

		template<typename Item>
		type_struct_vector(Item*) : type_descriptor("vector", sizeof(std::vector<Item>), TYPE_CLASSIFY::TYPE_STL_ARRAY),
			item_type(type_resolver<Item>::get()) {
			get_size = [](const void* vec_ptr) -> size_t {
				const auto& vec = *(const std::vector<Item>*)vec_ptr;
				return vec.size();
			};

			get_item = [](const void* vec_ptr, size_t index) -> const void* {
				const auto& vec = *(const std::vector<Item>*)vec_ptr;
				return &vec[index];
			};

			resize = [](const void* vec_ptr, size_t size) -> size_t {
				auto& vec = *(std::vector<Item>*)vec_ptr;
				vec.resize(size);
				return vec.size();
			};
		}

		virtual std::string full_name() const override {
			return std::string("vector<") + item_type->full_name() + ">";
		}

		virtual void serialize(file_parser* parser, const void* obj, int level) override {
			parser->write_primitive_array_type(item_type, get_item, get_size(obj), obj, level);
		}

		virtual void deserialize(file_parser* parser, std::string var, const void* obj, int level) override {
			auto& tmp = (*parser)[var];
			if (tmp.checked || tmp.value.empty()) {
				return;
			}
			tmp.checked = true;
			auto size = resize(obj, std::stoi(tmp.value));	
			for (size_t index = 0; index < size; ++index)
			{
				item_type->deserialize(parser, item_type->full_name(), get_item(obj, index), level);
			}
		}
	};

	template<typename T>
	struct type_resolver<std::vector<T>> {
		static type_descriptor* get() {
			static type_struct_vector type_vector((T*)(nullptr));
			return &type_vector;
		}
	};

	/* 描述数组类型 */
	struct type_struct_array : type_descriptor {
		using get_item_func_ptr = const void* (*)(const void*, size_t);
		
		size_t size;
		type_descriptor* item_type;
		get_item_func_ptr get_item;

		template<typename Item>
		type_struct_array(Item*, size_t capacity) : type_descriptor("array", sizeof(Item) * capacity, TYPE_CLASSIFY::TYPE_CUSTOMIZE_ARRAY), 
			item_type(type_resolver<Item>::get()), size(capacity) {
			
			get_item = [](const void* arr_ptr, size_t index) -> const void* {
				const auto arr = (const Item*)arr_ptr;
				return &arr[index];
			};
		}

		virtual std::string full_name() const override {
			return std::string("array<") + item_type->full_name() + ">";
		}

		virtual void serialize(file_parser* parser, const void* obj, int level) override {
			parser->write_primitive_array_type(item_type, get_item, size, obj, level);
		}

		virtual void deserialize(file_parser* parser, std::string var, const void* obj, int level) override {
			auto& tmp = (*parser)[var];
			if (tmp.checked || tmp.value.empty()) {
				return;
			}
			tmp.checked = true;
			for (size_t index = 0; index < size; ++index)
			{
				item_type->deserialize(parser, item_type->full_name(), get_item(obj, index), level);
			}
		}
	};

	template<typename T, size_t N>
	struct type_resolver<T[N]> {
		static type_descriptor* get() {
			static type_struct_array type_array((T*)nullptr, N);
			return &type_array;
		}
	};

	/* 反射的宏定义 */
#define REFLECT(type) \
	static ref::type_struct_base<type> reflection; \
	static void init_reflection(ref::type_struct_base<type>*); \
	/* 解析类 */
#define REFLECT_STRUCT_BEGIN(type) \
	ref::type_struct_base<type> type::reflection(type::init_reflection); \
	void type::init_reflection(ref::type_struct_base<type>* type_class) { \
		using T = type; \
		type_class->type_name = #type; \
		type_class->type_size = sizeof(T); \
		type_class->members = {

#define REFLECT_STRUCT_MEMBER(name) \
			{#name, offsetof(T, name), ref::type_resolver<decltype(T::name)>::get()},

#define REFLECT_STRUCT_END \
		}; \
	} \

	/* 内建类型 */
#define REFLECT_TYPE(type) \
	template<> \
	struct type_struct_base<type> : type_descriptor { \
		using type_class = typename type; \
		using type_pointer = typename type*; \
		type_struct_base() : type_descriptor(#type, sizeof(type), type_is_customize_v<type>) {} \
		virtual void serialize(ref::file_parser* parser, const void* obj, int) override { \
			parser->write_primitive_type(#type, *(const type*)obj); \
		} \
		virtual void deserialize(file_parser* parser, std::string var, const void* obj, int) override { \
			auto type_obj = (type_pointer)obj; \
			*type_obj = string_to_data<type_class>((*parser)[var].value); \
			(*parser)[var].checked = true; \
		} \
	}; \
	template<> \
	type_descriptor* get_primitive_desc<type>() { \
		static type_struct_base<type> type_##type; \
		return &type_##type; \
	} \

} // namespace reflect