#include <iostream>

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
    /* deserialize */
    {
        Node node;
        ref::type_descriptor* typeDesc = ref::type_resolver<Node>::get();

        std::string data = "Node { \n\
                key = string{ apple } \n\
                value = int{ 3 } \n\
                children = vector<Node>[2]{ \n\
                    Node { \n\
                        key = string{banana} \n\
                        value = int{7} \n\
                        children = vector<Node>[0]{} \n\
                    } \n\
                    Node { \n\
                        key = string{cherry} \n\
                        value = int{11} \n\
                        children = vector<Node>[0]{} \n\
                    } \n\
                } \n\
            }";

        ref::format_helper helper(data);

        typeDesc->deserialize("Node", &node, helper);
    }

    /* serialize */
    {
        Node node = { "apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}} };

        ref::type_descriptor* typeDesc = ref::type_resolver<Node>::get();

        typeDesc->serialize(&node);
    }

	return 0;
}