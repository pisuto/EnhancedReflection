#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ref_base.h"

namespace ref {

	struct type_descriptor;

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct format_helper {
		format_helper(std::string name) : file_name(name), first_write(true) {}

		template<typename T, typename ...Args>
		void read(T& data, Args&... args)
		{
			if (dic.empty()) {
				in.open(file_name, std::ios::in);
				file_read(in);
				in.close();
			}
			auto type_desc = ref::type_resolver<T>::get();
			type_desc->deserialize(type_desc->full_name(), &data, *this);
			read(args...);
		}

		template<typename ...Args>
		void read(Args&... args) {}

		template<>
		void read() {}

		template<typename T, typename ...Args>
		void write(T& data, Args&... args)
		{
			if (first_write) {
				out.open(file_name, std::ios::out);
				first_write = false;
			}
			auto type_desc = ref::type_resolver<T>::get();
			type_desc->serialize(out, &data);
			out << std::endl; /* 区分每一个类 */
			write(args...);
		}

		template<typename ...Args>
		void write(Args&... args) {}

		template<>
		void write()
		{
			first_write = true;
			out.close();
		}

		mapping& operator[](std::string index) { 
			for (auto it = dic.begin(); it != dic.end(); it++)
			{
				if (index == it->first && !it->second.checked) {
					return it->second;
				}
			}
			return dic.back().second; /* 需要返回一个值 */
		}

	private:
		std::vector<std::pair<std::string, mapping>> dic;
		std::string file_name;
		bool first_write;

		std::ofstream out;
		std::ifstream in;

		void file_read(std::ifstream& in);
	};
}