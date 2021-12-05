#include <iostream>
#include <fstream>

#include "reflect.h"


struct Node {
    std::string key;
    int value;
    std::vector<Node> children;

    REFLECT(Node)       
};

REFLECT_STRUCT_BEGIN(Node)
REFLECT_STRUCT_MEMBER(key)
REFLECT_STRUCT_MEMBER(value)
REFLECT_STRUCT_MEMBER(children)
REFLECT_STRUCT_END

int main()
{
    ref::format_helper helper("setting.ini");

    /* serialize */
    {
        Node node = { "apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}} };

        ref::type_descriptor* type_desc = ref::type_resolver<Node>::get();

        helper.write(type_desc, &node);
    }

    /* deserialize */
    {
        Node node;

        ref::type_descriptor* type_desc = ref::type_resolver<Node>::get();

        helper.read(type_desc, &node);
    }

	return 0;
}