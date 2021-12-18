#pragma once

#include <fstream>

#include "ref_traits.h"

namespace ref {

	struct file_parser;
	struct type_descriptor;

	struct type_member {
		const char* var_name;
		size_t offset;
		type_descriptor* type_desc;
	};

	/* 类型描述 */
	struct type_descriptor {
		type_descriptor(const char* name, size_t size, TYPE_CLASSIFY kind) :
			type_name(name), type_size(size), type_class(kind) {}
		virtual ~type_descriptor() {}
		virtual std::string full_name() const { return type_name; }
		virtual void type_members(std::vector<type_member>&) {}; /* override for customize struct */
		virtual void serialize(file_parser* parser, const void* obj, int level = 1) = 0;
		virtual void deserialize(file_parser* parser, std::string var, const void* obj, int level = 0) = 0;
		const char* type_name;
		size_t type_size;
		TYPE_CLASSIFY type_class;
	};

	/* string转数据 */
	template<typename T>
	T string_to_data(std::string);

	/* 内建类型获取 */
	template<typename T>
	type_descriptor* get_primitive_desc();

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
}