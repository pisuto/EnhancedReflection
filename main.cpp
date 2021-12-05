#include <iostream>

#include "reflect.h"


struct Node {
    std::string key;
    int value;

    REFLECT(Node)       
};

REFLECT_STRUCT_BEGIN(Node)
REFLECT_STRUCT_MEMBER(key)
REFLECT_STRUCT_MEMBER(value)
REFLECT_STRUCT_END

int main()
{
    Node node;

    ref::type_descriptor* typeDesc = ref::type_resolver<Node>::get();

    std::string data = "Node { \n\
        key = string{ apple } \n\
        value = int{ 3 } \n\
    } \n";
    
    ref::format_helper helper(data);
    typeDesc->deserialize("Node", &node, helper);

    std::cout << node.key << " " << node.value << std::endl;

	return 0;
}