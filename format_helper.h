#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace ref {

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct format_helper {
		format_helper(std::string data) {
			auto tmp = data;
			int pos = 0, level = 0;
			while ((pos = tmp.find_first_of("\n")) != std::string::npos)
			{
				auto line = tmp.substr(0, pos);
				std::string name;
				{
					auto pos1 = line.find_first_not_of(" ");
					auto pos2 = line.find_first_of(" ", pos1);
					name = line.substr(pos1, pos2 - pos1);
				}
				
				std::string value;
				{
					auto pos1 = line.find_first_of("{");
					auto pos2 = line.find_first_of("}");
					if (pos1 != std::string::npos && pos2 != std::string::npos)
					{
						value = line.substr(pos1 + 1, pos2 - pos1 - 1);
						auto pos1 = value.find_first_not_of(" ");
						auto pos2 = value.find_last_not_of(" ");
						if (pos1 == std::string::npos && pos2 == std::string::npos)
						{
							pos1 = line.find_first_of("[");
							pos2 = line.find_first_of("]");
							if (pos1 != std::string::npos && pos2 != std::string::npos)
							{
								value = line.substr(pos1 + 1, pos2 - pos1 - 1);
								dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
							}
							else
							{
								dic.emplace_back(std::make_pair(name, mapping{ "", level, false }));
							}
						}
						else 
						{
							value = value.substr(pos1, pos2 - pos1 + 1);
							dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
						}
					}
					else if (pos2 == std::string::npos)
					{
						pos1 = line.find_first_of("[");
						pos2 = line.find_first_of("]");
						if (pos1 != std::string::npos && pos2 != std::string::npos)
						{
							value = line.substr(pos1 + 1, pos2 - pos1 - 1);
							dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
						}
						else
						{
							dic.emplace_back(std::make_pair(name, mapping{ "", level, false }));
						}
						++level;
					}
				}
				tmp = tmp.substr(pos + 1, tmp.size() - pos - 1);
			}
		}

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
	};
}