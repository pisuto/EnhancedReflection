# EnhancedReflection
[![standard-readme compliant](https://img.shields.io/badge/csdn%20blog-link-brightgreen.svg?style=flat-square)](https://github.com/pisuto/EnhancedReflection)

An enhanced version of FlexibleReflection.
## Table of Contents
- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [Related Efforts](#related-efforts)
- [Author](#author)
- [License](#license)
## Background
The function provided by FlexibleReflection is serializing data but lacking of deserializing. Therefore, based on FlexibleReflection, this developping project is an enhanced version including deserializing function and so on.

## Install
This project is developped using VisualStudio2019. Clone the source code and create an C++ empty project, then copy these .h and .cpp files into your project.
```sh
$ git clone https://github.com/pisuto/EnhancedReflection.git
```

## Usage
Include the header file `ref_define.h` and enable reflection feature, for example `Node`.
```c++
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
```
After finishing defining the struct, then construct `ref::format_helper` and choose the specific parser tool for the file to be resolved. This project provides three simple parser `ref::xml_parser` `ref::txt_parser` `ref::console_parser`. Their names imply their corresponding formats. Finally, using member functions `write` and `read` provided by the helper achieves serializing and deserializing effects.
```c++
/* choose one of the parsers by the format of file */
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
```
***Noted: the order of these types when read must be consistent with these when wrote，otherwise the result will be abnormal.***


## Related Efforts
- [FlexibleReflection](https://github.com/preshing/FlexibleReflection) - a small, flexible runtime reflection system using C++11 language features.
- [TestCppReflect](https://github.com/tapika/TestCppReflect/) - test application for using C++ reflection

## Author
[@pisuto](https://github.com/pisuto).

## License
[MIT](LICENSE) © pisuto
