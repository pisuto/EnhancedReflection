#pragma once

#include <string>
#include <fstream>

namespace ref {

	struct format_helper;

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

	/* 类型描述 */
	struct type_descriptor {
		type_descriptor(const char* name, size_t size) :
			type_name(name), type_size(size) {}
		virtual ~type_descriptor() {}
		virtual std::string full_name() const { return type_name; }
		virtual void serialize(std::ofstream& out, const void* obj, int level = 0) const = 0;
		virtual void deserialize(std::string var, const void* obj, format_helper& helper, int level = 0) = 0;

		const char* type_name;
		size_t type_size;
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