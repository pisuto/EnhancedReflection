#include "ref_define.h"

using namespace std;

namespace ref {

	template<>
	int string_to_data<int>(std::string data) {
		return data.empty()? 0 : std::stoi(data);
	}

	template<>
	std::string string_to_data<std::string>(std::string data) {
		return data;
	}

	template<>
	bool string_to_data<bool>(std::string data) {
		return data == "true";
	}

	REFLECT_TYPE(int)
	REFLECT_TYPE(bool)
	REFLECT_TYPE(string)

}