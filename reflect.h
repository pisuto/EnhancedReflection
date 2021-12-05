#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <cstddef>

#include "format_helper.h"

namespace ref {

	/* 前置声明 */
	struct type_descriptor;

	/* 工具 */
	template<bool B, typename T = void>
	struct type_enable_if {};

	template<typename T>
	struct type_enable_if<true, T> {
		using type = T;
	};

	template<bool B, typename T = void>
	using type_enable_if_t = typename type_enable_if<B, T>::type;

	template<typename T>
	struct type_is_reflected {
		using Yes = char;
		struct No { char dummy[2]; };
		template<typename Y>
		static Yes test(decltype(&Y::reflection));
		template<typename Y>
		static No test(...);

		enum { value = (sizeof(test<T>(nullptr)) == sizeof(Yes)) };
	};

	/* string转数据 */
	template<typename T>
	T string_to_data(std::string);

	/* 内建类型获取 */
	template<typename T>
	type_descriptor* get_primitive_desc();

	/* 类型描述 */
	struct type_descriptor {
		type_descriptor(const char* name, size_t size) :
			type_name(name), type_size(size) {}
		virtual ~type_descriptor() {}
		virtual std::string full_name() const { return type_name; }
		virtual void serialize(const void* obj, int level = 0) const = 0;
		virtual void deserialize(std::string var, void* obj, format_helper& helper, int level = 0) = 0;
		
		const char* type_name;
		size_t type_size;
	};

	/* 解析类的父类 */
	template<typename T>
	struct type_struct_base : type_descriptor {
		using type_class    = typename T;
		using type_pointer  = typename T*;
		using init_func_ptr = void(*)(type_struct_base*);

		struct member {
			const char* var_name;
			size_t offset;
			type_descriptor* type_desc;
		};

		std::vector<member> members;

		type_struct_base(init_func_ptr init) : type_descriptor(nullptr, 0) {
			init(this);
		}
		type_struct_base(const char* name, size_t size, const std::initializer_list<member>& init) : type_descriptor(nullptr, 0),
			members(init) {}

		virtual void serialize(const void* obj, int level) const override {
			std::cout << full_name() << " {" << std::endl;
			for (const auto& element : members)
			{
				std::cout << std::string(4 * (level + 1), ' ') << element.var_name << " = ";
				element.type_desc->serialize((char*)obj + element.offset, level + 1);
				std::cout << std::endl;
			}
			std::cout << std::string(4 * level, ' ') << "}";
		}

		virtual void deserialize(std::string var, void* obj, format_helper& helper, int level) override {
			for (auto& element : members)
			{
				element.type_desc->deserialize(element.var_name, (char*)obj + element.offset, helper , level + 1);
			}
		}
	};

	/* 类型解析器，返回类型描述 */
	template<typename T>
	struct type_resolver {

		template<typename Y = T /* ? */, typename type_enable_if_t<type_is_reflected<Y>::value, int> = 0>
		static type_descriptor* get() {
			return &Y::reflection;
		}

		// 默认的解析器
		template<typename Y = T, typename type_enable_if_t<!type_is_reflected<Y>::value, int> = 0>
		static type_descriptor* get() {
			return get_primitive_desc<Y>();
		}

	};

	/* vector */
	struct type_struct_vector : type_descriptor {
		using get_size_func_ptr = size_t(*)(const void*);
		using get_item_func_ptr = const void* (*)(const void*, size_t);
		get_size_func_ptr get_size;
		get_item_func_ptr get_item;
		type_descriptor* item_type;

		template<typename Item>
		type_struct_vector(Item*) : type_descriptor("vector", sizeof(std::vector<Item>)),
			item_type(type_resolver<Item>::get()) {
			get_size = [](const void* vec_ptr) -> size_t {
				const auto& vec = *(const std::vector<Item>*)vec_ptr;
				return vec.size();
			};

			get_item = [](const void* vec_ptr, size_t index) -> const void* {
				const auto& vec = *(const std::vector<Item>*)vec_ptr;
				return &vec[index];
			};
		}

		virtual std::string full_name() const override {
			return std::string("vector<") + item_type->full_name() + ">";
		}

		virtual void serialize(const void* obj, int level) const override {
			auto size = get_size(obj);
			std::cout << full_name();
			if (size == 0) {
				std::cout << "{}";
			}
			else {
				std::cout << "{" << std::endl;
				for (size_t index = 0; index < size; ++index)
				{
					std::cout << std::string(4 * (level + 1), ' ') << "[" << index << "] ";
					item_type->serialize(get_item(obj, index), level + 1);
					std::cout << std::endl;
				}
				std::cout << std::string(4 * level, ' ') << "}";
			}
		}

		virtual void deserialize(std::string var, void* obj, format_helper& helper, int level) override {

		}
	};

	template<typename T>
	struct type_resolver<std::vector<T>> {
		static type_descriptor* get() {
			static type_struct_vector type_vector((T*)(nullptr));
			return &type_vector;
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
		type_struct_base() : type_descriptor(#type, sizeof(type)) {} \
		virtual void serialize(const void* obj, int) const override { \
			std::cout << #type << "{" << *(const type*)obj << "}"; \
		} \
		virtual void deserialize(std::string var, void* obj, format_helper& helper, int) override { \
			auto type_obj = (type_pointer)obj; \
			*type_obj = string_to_data<type_class>(helper[var].value); \
		} \
	}; \
	template<> \
	type_descriptor* get_primitive_desc<type>() { \
		static type_struct_base<type> type_##type; \
		return &type_##type; \
	} \

} // namespace reflect