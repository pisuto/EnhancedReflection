#include "reflect.h"

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

	REFLECT_TYPE(int)
	REFLECT_TYPE(string)

}