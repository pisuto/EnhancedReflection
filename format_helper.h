#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace ref {

	struct type_descriptor;

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct format_helper {
		format_helper(std::string name) : file_name(name) {}

		void read(type_descriptor* type_desc, const void* data);

		void write(type_descriptor* type_desc, const void* data);

		mapping& operator[](std::string index) { 
			for (auto it = dic.begin(); it != dic.end(); it++)
			{
				if (index == it->first && !it->second.checked) {
					return it->second;
				}
			}
			return dic[0].second; /* 需要返回一个值 */
		}

	private:
		std::vector<std::pair<std::string, mapping>> dic;
		std::string file_name;

		void file_read(std::ifstream& in);
	};
}