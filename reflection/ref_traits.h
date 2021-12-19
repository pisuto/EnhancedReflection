#pragma once

#include <string>
#include <list>

namespace ref {

	constexpr static const char* NULL_VALUE = "0";

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

	enum class  TYPE_CLASSIFY {
		TYPE_NULL,
		TYPE_CUSTOMIZE,        /* 自定义类型 */
		TYPE_CUSTOMIZE_ARRAY,  /* 自定义数组 */
		TYPE_PRIMITIVE,        /* 内置类型 */
		TYPE_STL,              /* STL类型 */
		TYPE_STL_ARRAY,        /* STL容器 */
	};

	template<typename T>
	struct type_is_customize {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_CUSTOMIZE;
	};

	template<>
	struct type_is_customize<std::string> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_STL;
	};

	template<>
	struct type_is_customize<unsigned char> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<int> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<char> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<size_t> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<float> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<bool> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<>
	struct type_is_customize<double> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_PRIMITIVE;
	};

	template<typename T>
	struct type_is_customize<std::vector<T>> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_STL_ARRAY;
	};

	template<typename T>
	struct type_is_customize<std::list<T>> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_STL_ARRAY;
	};

	template<typename T, size_t N>
	struct type_is_customize<T[N]> {
		constexpr static auto value = TYPE_CLASSIFY::TYPE_CUSTOMIZE_ARRAY;
	};

	template<typename T>
	constexpr static TYPE_CLASSIFY type_is_customize_v = type_is_customize<T>::value;
}