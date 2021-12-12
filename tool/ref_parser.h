#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <list>
#include <functional>

namespace ref {

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct file_parser {
		file_parser(std::string name) : file_name(name), 
			is_read(false), is_wrote(false) {}
		
		virtual bool is_read_file() const { return is_read; }
		virtual bool is_write_file() const { return is_wrote; }

		virtual void read_file() = 0;
		virtual void write_file() = 0;
		virtual void write_file_finished() = 0;
		virtual void write_class_name(std::string, int, int size = -1) = 0;
		virtual void write_member_name(std::string, std::string, int) = 0;
		virtual void write_ident_space(int level) = 0;
		virtual void write_enter() = 0;
		virtual void write_class_end(std::string) = 0;
		virtual mapping& operator[](std::string) = 0;

		/* 根据不同的类型变化传值，子类重写不了父类的模板函数，因此只能一一写出 */
		virtual void write_member_value(std::string type_name, int type_value) = 0;
		virtual void write_member_value(std::string type_name, std::string type_value) = 0;
		virtual void write_member_value(std::string type_name, bool type_value) = 0;

		std::string file_name;
		bool is_read, is_wrote;
	};

	/* .ini .txt文件读入 */
	class txt_parser : public file_parser {
	public:
		txt_parser(std::string name) : file_parser(name) {}

		virtual void write_class_name(std::string class_name, int, int size = -1) override {
			out << class_name;
			if (size < 0) {
				out << " {" << std::endl;
			}
			/* 为确保vector的大小，解析时用resize */
			else if (size == 0) {
				out << "[" << 0 << "]" << " {}";
			}
			else {
				out << "[" << size << "]" << " {" << std::endl;
			}
		}

		virtual void write_member_name(std::string var_name, std::string class_name, int level) override {
			out << std::string(4 * (level), ' ') << var_name << " = ";
		}

		virtual void write_ident_space(int level) override {
			out << std::string(4 * (level), ' ');
		}

		virtual void write_enter() override {
			out << std::endl;
		}

		virtual void write_class_end(std::string) override {
			out << "}";
		}

		virtual void write_member_value(std::string type_name, int type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, std::string type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, bool type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		template<typename T>
		void write_member_value_for_base(std::string type_name, T type_value) {
			out << type_name << "{" << std::boolalpha << type_value << "}";
		}

		/* 粗糙地解析文件，没有对文件格式进行判断 */
		virtual void read_file() override {
			in.open(file_name, std::ios::in);
			std::string line;
			int level = 0;
			while (std::getline(in, line))
			{
				std::string name;
				{
					auto pos1 = line.find_first_not_of(" ");
					auto pos2 = line.find_first_of(" ", pos1);
					name = line.substr(pos1, pos2 - pos1);
				}

				std::string value;
				{
					auto pos1 = line.find_first_of("{");
					auto pos2 = line.find_first_of("}");
					if (pos1 != std::string::npos && pos2 != std::string::npos)
					{
						value = line.substr(pos1 + 1, pos2 - pos1 - 1);
						auto pos1 = value.find_first_not_of(" ");
						auto pos2 = value.find_last_not_of(" ");
						if (pos1 == std::string::npos && pos2 == std::string::npos)
						{
							pos1 = line.find_first_of("[");
							pos2 = line.find_first_of("]");
							if (pos1 != std::string::npos && pos2 != std::string::npos)
							{
								value = line.substr(pos1 + 1, pos2 - pos1 - 1);
								dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
							}
							else
							{
								dic.emplace_back(std::make_pair(name, mapping{ "", level, false }));
							}
						}
						else
						{
							value = value.substr(pos1, pos2 - pos1 + 1);
							dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
						}
					}
					else if (pos2 == std::string::npos)
					{
						pos1 = line.find_first_of("[");
						pos2 = line.find_first_of("]");
						if (pos1 != std::string::npos && pos2 != std::string::npos)
						{
							value = line.substr(pos1 + 1, pos2 - pos1 - 1);
							dic.emplace_back(std::make_pair(name, mapping{ value, level, false }));
						}
						else
						{
							dic.emplace_back(std::make_pair(name, mapping{ "", level, false }));
						}
						++level;
					}
				}
			}
			/* 防止取不出数据没有返回值，因此加入一个null，但一般用不到 */
			dic.emplace_back(std::make_pair("null", mapping{ "", 0, false }));
			in.close();
			is_read = true;
		}

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

		virtual void write_class_name(std::string class_name, int, int size = -1) override {
			std::cout << class_name;
			if (size < 0) {
				std::cout << " {" << std::endl;
			}
			else if (size == 0) {
				std::cout << "[" << 0 << "]" << " {}";
			}
			else {
				std::cout << "[" << size << "]" << " {" << std::endl;
			}
		}

		virtual void write_member_name(std::string var_name, std::string class_name, int level) override {
			std::cout << std::string(4 * (level), ' ') << var_name << " = ";
		}

		virtual void write_ident_space(int level) override {
			std::cout << std::string(4 * (level), ' ');
		}

		virtual void write_enter() override {
			std::cout << std::endl;
		}

		virtual void write_class_end(std::string) override {
			std::cout << "}";
		}

		virtual void write_member_value(std::string type_name, int type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, std::string type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, bool type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		template<typename T>
		void write_member_value_for_base(std::string type_name, T type_value) {
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
		
		XML_STRU_TYPE get_xml_type(std::string& line, xml_doc& doc) {
			if (line.empty()) {
				return XML_STRU_TYPE::XML_NULL;
			}
			xml_node node;
			auto las /* left arrow start */ = line.find_first_of('<');
			if (las != std::string::npos) {
				if (las + 1 < line.length() && line[las + 1] == '?') {
					return XML_STRU_TYPE::XML_DECL;
				}
				auto lae /* left arrow end */ = line.find_first_of('>', las);
				if (lae != std::string::npos) {
					if (line[las + 1] != '/') {
						node.text.name = line.substr(las + 1, lae - las - 1);
						auto ras /* right arrow start */ = line.find_first_of("</", lae);
						if (ras != std::string::npos) {
							auto rae /* right arrow end */ = line.find_first_of('>', ras);
							if (rae != std::string::npos) {
								/* 读取到<>...</>节点时将其压入上一个父节点 */
								auto node_name = line.substr(ras + 2, rae - ras - 2);
								if (node_name == node.text.name) {
									std::string value = lae + 1 != ras? line.substr(lae + 1, ras - lae - 1) : NULL_VALUE;
									node.text.value = { value, 0, false };
									xml_store_nodes.top().xml_nodes.push_back(node);
									return XML_STRU_TYPE::XML_NODE;
								}
								return XML_STRU_TYPE::XML_ERROR_WRONG_NODE_NAME;
							}
						}
						else {
							/* 当碰到<>标签时创建新的节点压入栈 */
							xml_store_nodes.push(node);
							return XML_STRU_TYPE::XML_HEAD;
						}
					}
					else {
						/* 碰到</>标签时将最后一个节点弹出并压入父节点 */
						auto temp = xml_store_nodes.top();
						node.text.name = line.substr(las + 2, lae - las - 2);
						node.text.value.level = xml_store_nodes.size() - 1;
						if (temp.text.name == node.text.name) {
							if (xml_store_nodes.size() == 1) {
								doc.xml_nodes.push_back(xml_store_nodes.top());
							}
							xml_store_nodes.pop();
							if (!xml_store_nodes.empty()) {
								auto& top = xml_store_nodes.top();
								top.xml_nodes.push_back(temp);
								top.text.value.value = std::to_string(top.xml_nodes.size());
								top.text.value.level = temp.text.value.level + 1;
							}
							return XML_STRU_TYPE::XML_TAIL;
						}
						return XML_STRU_TYPE::XML_ERROR_WRONG_NODE_NAME;
					}
				}
				else {
					return XML_STRU_TYPE::XML_ERROR_NO_RIGHT_ARROR;
				}
			}
			return XML_STRU_TYPE::XML_ERROR_UNKNOWN;
		}

		std::stack<xml_node> xml_store_nodes;
	};

	/* xml文件读入 */
	class xml_parser : public file_parser {
	public:
		xml_parser(std::string name) : file_parser(name) {
			null_node = {};
			null_node.text.value.level = -1;
		}

		virtual void write_class_name(std::string class_name, int level, int size = -1) override {
			if (check_if_array(class_name, [&]() {
				if (size == 0) {
					out << "</" << end_arr_names.top() << ">";
					end_arr_names.pop();
				}
				else {
					out << std::endl;
				}
				})) {
				return;
			}
			out << "<" << class_name << ">" << std::endl;
		}

		virtual void write_member_name(std::string var_name, std::string class_name, int level) override {
			out << std::string(4 * (level), ' ') << "<" << var_name << ">";
			end_var_name = var_name;
			check_if_array(class_name, [&]() {
				end_arr_names.push(end_var_name);
				});
		}

		virtual void write_ident_space(int level) override {
			out << std::string(4 * (level), ' ');
		}

		virtual void write_enter() override {
			out << std::endl;
		}

		virtual void write_class_end(std::string class_name) override {
			if (check_if_array(class_name, [&]() {
				out << "</" << end_arr_names.top() << ">";
				end_arr_names.pop();
				})) {
				return;
			}
			out << "</" << class_name << ">";
		}

		virtual void write_member_value(std::string type_name, int type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, std::string type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		virtual void write_member_value(std::string type_name, bool type_value) {
			write_member_value_for_base(type_name, type_value);
		}

		template<typename T>
		void write_member_value_for_base(std::string type_name, T type_value) {
			out << std::boolalpha << type_value << "</" << end_var_name << ">";
			end_var_name.clear();
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
			out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
			is_wrote = true;
			is_read = false;
		}

		virtual void write_file_finished() override {
			out.close();
			is_wrote = false;
			if (check_if_valid_xml_finished()) {
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
		bool check_if_array(std::string class_name, std::function<void()> print) {
			if (class_name.find('<') != std::string::npos &&
				class_name.find('>') != std::string::npos) {
				print();
				return true;
			}
			return false;
		}

		bool check_if_valid_xml_finished() {
			return end_arr_names.empty() && end_var_name.empty();
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

		std::string end_var_name;
		std::stack<std::string> end_arr_names;

		xml_node null_node;
	};
}