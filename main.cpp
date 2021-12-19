#include <iostream>
#include <fstream>

#include "reflection/ref_define.h"

struct Size
{
    size_t width;
    size_t height;

    REFLECT(Size)
};

REFLECT_STRUCT_BEGIN(Size)
REFLECT_STRUCT_MEMBER(width)
REFLECT_STRUCT_MEMBER(height)
REFLECT_STRUCT_END

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
    std::vector<Node> nodes1;
    Node nodes2[3];
    std::vector<std::string> nodes3;
    std::vector<Size> nodes4;

    REFLECT(Base)
};

REFLECT_STRUCT_BEGIN(Base)
REFLECT_STRUCT_MEMBER(checked)
REFLECT_STRUCT_MEMBER(nodes1)
REFLECT_STRUCT_MEMBER(nodes2)
REFLECT_STRUCT_MEMBER(nodes3)
REFLECT_STRUCT_MEMBER(nodes4)
REFLECT_STRUCT_END

int main()
{
    ref::format_helper helper(new ref::xml_parser("setting.xml"));

    /* serialize */
    {
        Base base = { false, {{"water", 4, {}}}, {{"orange", 5, {}}, {"pear", 6, {}}, {}},
            {"strawberry", "peach"}, {{{3,4}, {5,6}}} };

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

