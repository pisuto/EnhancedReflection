#include <iostream>
#include <fstream>

#include "ref_define.h"


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

struct Base {
    bool checked;
    std::vector<Node> nodes;

    REFLECT(Base)
};

REFLECT_STRUCT_BEGIN(Base)
REFLECT_STRUCT_MEMBER(checked)
REFLECT_STRUCT_MEMBER(nodes)
REFLECT_STRUCT_END

int main()
{
    ref::format_helper helper("setting.ini");

    /* serialize */
    {
        Base base = { false, {{"water", 4, {}}}};

        Node node = { "apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}} };

        ref::type_descriptor* type_desc = ref::type_resolver<Node>::get();

        helper.write(base, node);
    }

    /* deserialize */
    
        Base base;

        Node node;

        ref::type_descriptor* type_desc = ref::type_resolver<Node>::get();

        helper.read(base, node);
    

	return 0;
}