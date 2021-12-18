#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <list>
#include <functional>

namespace ref {

	struct type_descriptor;

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct file_parser {
		using get_item_func_ptr = const void* (*)(const void*, size_t);

		file_parser(std::string name) : file_name(name), 
			is_read(false), is_wrote(false) {}
		
		virtual bool is_read_file() const { return is_read; }
		virtual bool is_write_file() const { return is_wrote; }

		virtual void read_file() = 0;
		virtual void write_file() = 0;
		virtual void write_file_finished() = 0;
		virtual void write_customize_type(type_descriptor*, const void*, int) = 0;
		virtual void write_primitive_array_type(type_descriptor*, get_item_func_ptr, int, const void*, int) = 0;
		
		virtual mapping& operator[](std::string) = 0;

		/* 根据不同的类型变化传值，子类重写不了父类的模板函数，因此只能一一写出 */
		virtual void write_primitive_type(std::string type_name, int type_value) = 0;
		virtual void write_primitive_type(std::string type_name, std::string type_value) = 0;
		virtual void write_primitive_type(std::string type_name, bool type_value) = 0;
		virtual void write_primitive_type(std::string type_name, size_t type_value) = 0;

		std::string file_name;
		bool is_read, is_wrote;
	};

	/* .ini .txt文件读入 */
	class txt_parser : public file_parser {
	public:
		txt_parser(std::string name) : file_parser(name) {}

		virtual void write_customize_type(type_descriptor*, const void*, int) override;

		virtual void write_primitive_array_type(type_descriptor*, get_item_func_ptr, int, const void*, int) override;

		virtual void write_primitive_type(std::string type_name, int type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, std::string type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, bool type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, size_t type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		template<typename T>
		void write_primitive_type_template(std::string type_name, T type_value) {
			out << type_name << "{" << std::boolalpha << type_value << "}";
		}

		/* 粗糙地解析文件，没有对文件格式进行判断 */
		virtual void read_file() override;

		virtual void write_file() override {
			dic.clear(); /* 每一次读入时就清空，需要重新读入 */
			out.open(file_name, std::ios::out);
			is_wrote = true;
			is_read = false;
		}

		virtual void write_file_finished() override {
			out.close();
			is_wrote = false;
		}

		/* 按从最开始到最后加入的顺序读取数据 */
		mapping& operator[](std::string index) {
			for (auto it = dic.begin(); it != dic.end(); it++)
			{
				if (index == it->first && !it->second.checked) {
					return it->second;
				}
			}
			return dic.back().second; /* 需要返回一个值 */
		}

	private:
		std::ifstream in;
		std::ofstream out;

		std::vector<std::pair<std::string, mapping>> dic;
	};

	
	/* console输出，没有读入功能 */
	struct console_parser : file_parser {
		console_parser() : file_parser("") {}

		virtual void write_customize_type(type_descriptor*, const void*, int) override;
		
		virtual void write_primitive_array_type(type_descriptor*, get_item_func_ptr, int, const void*, int) override;

		virtual void write_primitive_type(std::string type_name, int type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, std::string type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, bool type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, size_t type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		template<typename T>
		void write_primitive_type_template(std::string type_name, T type_value) {
			std::cout << type_name << "{" << std::boolalpha << type_value << "}";
		}

		virtual bool is_read_file() const override {
			std::cout << "ERROR: Cannot read this type, please use other parsers." << std::endl;
			return true;
		}

		/* 控制台读不了数据 */
		virtual void read_file() override {}

		virtual void write_file() override {}

		virtual void write_file_finished() override {}

		mapping& operator[](std::string index) {
			static mapping null_map = { "",0,false };
			return null_map; /* 需要返回一个值 */
		}
	};

	/* xml结构体，不存储attr，每一个节点只认为可能有子节点和text，简化读取 */
	enum class XML_STRU_TYPE {
		/* 错误码 */
		XML_ERROR_NO_RIGHT_ARROR,
		XML_ERROR_WRONG_NODE_NAME,
		XML_ERROR_UNKNOWN,

		/* 节点值 */
		XML_NULL,		/* 空节点 */
		XML_DOC,		/* 初始节点 */
		XML_HEAD,		/* 头节点，'<node>' */
		XML_TAIL,		/* 尾节点，'</node>' */
		XML_NODE,       /* 中间节点，'<node></node>' */
		XML_PCDATA,		/* 数据，'<node>text</node>' */
		XML_CDATA,		/* <![CDATA[text]]> */
		XML_COMMENT,	/* '<!-- text -->' */
		XML_DECL,		/* '<?xml version="1.0"?>' */
	};

	/* level在xml中由最底层开始递增 */
	struct xml_text {
		xml_text() : name(""), value{ "", 0, false } {}
		std::string name;
		mapping value;
	};

	struct xml_node {
		xml_text text;
		std::list<xml_node> xml_nodes;

		/* 用于和定义的null_node进行比较 */
		bool operator==(const xml_node& rhs) {
			if (text.name == rhs.text.name &&
				text.value.value == rhs.text.value.value &&
				text.value.level == rhs.text.value.level &&
				text.value.checked == rhs.text.value.checked &&
				xml_nodes.size() == rhs.xml_nodes.size()) {
				return true;
			}
			return false;
		}

		bool operator!=(const xml_node& rhs) {
			return !(*this == rhs);
		}
	};

	struct xml_doc {
		std::list<xml_node> xml_nodes;
	};

	struct xml_tiny_tool {

		void parser_xml(std::string& line, xml_doc& doc) {
			auto type = get_xml_type(line, doc);
			switch (type)
			{
			case ref::XML_STRU_TYPE::XML_ERROR_NO_RIGHT_ARROR:
				std::cout << "ERROR: .xml file lacks right arrow!" << std::endl;
				break;
			case ref::XML_STRU_TYPE::XML_ERROR_WRONG_NODE_NAME:
				std::cout << "ERROR: .xml file has error tag!" << std::endl;
				break;
			case ref::XML_STRU_TYPE::XML_ERROR_UNKNOWN:
				std::cout << "ERROR: .xml file encounters unknown error!" << std::endl;
				break;
			case ref::XML_STRU_TYPE::XML_NULL:
			case ref::XML_STRU_TYPE::XML_DOC:
			case ref::XML_STRU_TYPE::XML_HEAD:
			case ref::XML_STRU_TYPE::XML_TAIL:
			case ref::XML_STRU_TYPE::XML_NODE:
			case ref::XML_STRU_TYPE::XML_PCDATA:
			case ref::XML_STRU_TYPE::XML_CDATA:
			case ref::XML_STRU_TYPE::XML_COMMENT:
			case ref::XML_STRU_TYPE::XML_DECL:
			default:
				break;
			}
		}

	private:

		XML_STRU_TYPE get_xml_type(std::string& line, xml_doc& doc);

		std::stack<xml_node> xml_store_nodes;
	};

	/* xml文件读入 */
	class xml_parser : public file_parser {
	public:
		xml_parser(std::string name) : file_parser(name), tmp_var_name(""), is_array(false) {
			null_node = {};
			null_node.text.value.level = -1;
		}

		virtual void write_customize_type(type_descriptor*, const void*, int) override;
		
		virtual void write_primitive_array_type(type_descriptor*, get_item_func_ptr, int, const void*, int) override;

		virtual void write_primitive_type(std::string type_name, int type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, std::string type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, bool type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		virtual void write_primitive_type(std::string type_name, size_t type_value) {
			write_primitive_type_template(type_name, type_value);
		}

		template<typename T>
		void write_primitive_type_template(std::string type_name, T type_value) {
			out << std::boolalpha << type_value;
		}

		virtual void read_file() override {
			in.open(file_name, std::ios::in);
			std::string line;
			xml_tiny_tool tool;
			while (std::getline(in, line)) {
				tool.parser_xml(line, xml_file);
			}
			in.close();
			is_read = true;
			return;
		}

		virtual void write_file() override {
			// dic.clear(); /* 每一次读入时就清空，需要重新读入 */
			out.open(file_name, std::ios::out);
			out << "<?xml version=\"1.0\"?>" << std::endl << "<reflection>" << std::endl;
			is_wrote = true;
			is_read = false;
		}

		virtual void write_file_finished() override {
			out << "</reflection>" << std::endl;
			out.close();
			is_wrote = false;
			if (!check_if_valid_xml_finished()) {
				std::cout << "ERROR: Output .xml file has formal error!" << std::endl;
			}
		}

		mapping& operator[](std::string index) {
			for (auto& node : xml_file.xml_nodes) {
				auto& result = check_if_unchecked_node(index, node);
				if (result != null_node) {
					return result.text.value;
				}
			}
			return null_node.text.value;
		}

	private:

		bool check_if_valid_xml_finished() {
			return end_arr_names.empty();
		}

		/* 使用递归的方法进行遍历 */
		xml_node& check_if_unchecked_node(std::string& index, xml_node& node) {
			if (!node.text.value.checked && !node.text.value.value.empty() && index == node.text.name) {
				return node;
			}
			for (auto& sub : node.xml_nodes) {
				auto& result = check_if_unchecked_node(index, sub);
				if (result != null_node) {
					return result;
				}
			}
			return null_node;
		}

		std::ifstream in;
		std::ofstream out;
		xml_doc xml_file;

		std::stack<std::string> end_arr_names;

		std::string tmp_var_name;
		bool is_array;

		xml_node null_node;
	};
}