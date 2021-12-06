#pragma once

#include <vector>
#include <iostream>
#include <map>
#include <cstddef>

#include "ref_base.h"
#include "ref_helper.h"

namespace ref {

	/* ������ĸ��� */
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

		virtual void serialize(std::ofstream& out, const void* obj, int level) const override {
			out << full_name() << " {" << std::endl;
			for (const auto& element : members)
			{
				out << std::string(4 * (level + 1), ' ') << element.var_name << " = ";
				element.type_desc->serialize(out, (char*)obj + element.offset, level + 1);
				out << std::endl;
			}
			out << std::string(4 * level, ' ') << "}";
		}

		virtual void deserialize(std::string var, const void* obj, format_helper& helper, int level) override {
			for (auto& element : members)
			{
				element.type_desc->deserialize(element.var_name, (char*)obj + element.offset, helper , level + 1);
			}
		}
	};

	/* ������������࣬��������vector */
	struct type_struct_vector : type_descriptor {
		using get_size_func_ptr = size_t(*)(const void*);
		using get_item_func_ptr = const void* (*)(const void*, size_t);
		using resize_func_ptr   = size_t(*)(const void*, size_t);
		get_size_func_ptr get_size;
		get_item_func_ptr get_item;
		resize_func_ptr   resize;
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

			resize = [](const void* vec_ptr, size_t size) -> size_t {
				auto& vec = *(std::vector<Item>*)vec_ptr;
				vec.resize(size);
				return vec.size();
			};
		}

		virtual std::string full_name() const override {
			return std::string("vector<") + item_type->full_name() + ">";
		}

		virtual void serialize(std::ofstream& out, const void* obj, int level) const override {
			auto size = get_size(obj);
			out << full_name() << "[" + std::to_string(size) + "]";
			if (size == 0) {
				out << "{}";
				return;
			}

			out << "{" << std::endl;
			for (size_t index = 0; index < size; ++index)
			{
				out << std::string(4 * (level + 1), ' ');
				item_type->serialize(out, get_item(obj, index), level + 1);
				out << std::endl;
			}
			out << std::string(4 * level, ' ') << "}";
		}

		virtual void deserialize(std::string var, const void* obj, format_helper& helper, int level) override {
			auto& tmp = helper[var];
			if (tmp.checked || tmp.value.empty()) {
				return;
			}
			tmp.checked = true;
			auto size = resize(obj, std::stoi(tmp.value));	
			for (size_t index = 0; index < size; ++index)
			{
				item_type->deserialize(item_type->full_name(), get_item(obj, index), helper, level);
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

	/* ����ĺ궨�� */
#define REFLECT(type) \
	static ref::type_struct_base<type> reflection; \
	static void init_reflection(ref::type_struct_base<type>*); \
	/* ������ */
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

	/* �ڽ����� */
#define REFLECT_TYPE(type) \
	template<> \
	struct type_struct_base<type> : type_descriptor { \
		using type_class = typename type; \
		using type_pointer = typename type*; \
		type_struct_base() : type_descriptor(#type, sizeof(type)) {} \
		virtual void serialize(std::ofstream& out, const void* obj, int) const override { \
			out << #type << "{" << boolalpha  << *(const type*)obj << "}"; \
		} \
		virtual void deserialize(std::string var, const void* obj, format_helper& helper, int) override { \
			auto type_obj = (type_pointer)obj; \
			*type_obj = string_to_data<type_class>(helper[var].value); \
			helper[var].checked = true; \
		} \
	}; \
	template<> \
	type_descriptor* get_primitive_desc<type>() { \
		static type_struct_base<type> type_##type; \
		return &type_##type; \
	} \

} // namespace reflect