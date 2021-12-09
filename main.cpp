#include <iostream>
#include <fstream>

#include "ref_define.h"


struct Node {
    std::string key;
    int value;
    std::vector<Node> children;

    REFLECT(Node) // enable reflection     
};

// define related type descriptor
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
    ref::format_helper helper(new ref::xml_parser("setting.xml"));

    /* serialize */
    {
        Base base = { false, {{"water", 4, {}}}};

        Node node = { "apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}} };

        helper.write(base, node);
    }

    /* deserialize */
    {
        Base base;

        Node node;

        helper.read(base, node);
    }

	return 0;
}

