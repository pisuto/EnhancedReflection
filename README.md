# EnhancedReflection
[![standard-readme compliant](https://img.shields.io/badge/csdn%20blog-link-brightgreen.svg?style=flat-square)](https://github.com/pisuto/EnhancedReflection)

An enhanced version of flexible reflection.
## Table of Contents
- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [Related Efforts](#related-efforts)
- [Author](#author)
- [License](#license)
## Background
The function provided by FlexibleReflection is just serializing data and it's lacking of deserializing. Therefore, based on FlexibleReflection, this developping project is a enhanced version including deserializing function and so on.

## Install
This project is developped using visual studio 2019. Just clone and create an C++ empty project, then copy these .h and .cpp files into your project. Finally make fun!
```sh
$ git clone https://github.com/pisuto/EnhancedReflection.git
```

## Usage
Including the header files `ref_define.h` and define your struct, for example `Node`.
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
After finishing defining the struct, then just construct `ref::format_helper` and set the filename `setting.ini` that you can change to what you want. Finally, using member functions provided by helper `write` and `read` achieve serializing and deserializing effects.
```c++
ref::format_helper helper("setting.ini");

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
***Noted: the order of these types when read must be consistent with these when wrote， otherwise the result will be abnormal.***


## Related Efforts
- [FlexibleReflection](https://github.com/preshing/FlexibleReflection) - a small, flexible runtime reflection system using C++11 language features.
- [TestCppReflect](https://github.com/tapika/TestCppReflect/) - test application for using C++ reflection

## Author
[@pisuto](https://github.com/pisuto).

## License
[MIT](LICENSE) © pisuto
