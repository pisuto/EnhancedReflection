#include "ref_parser.h"

namespace ref {

	void txt_parser::read_file()
	{
		in.open(file_name, std::ios::in);
		std::string line;
		int level = 0;
		while (std::getline(in, line))
		{
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
		}
		dic.emplace_back(std::make_pair("null", mapping{ "", 0, false }));
		in.close();
		is_read = true;
	}

}