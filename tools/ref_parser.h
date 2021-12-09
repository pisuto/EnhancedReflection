#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
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

		virtual void write_member_value(std::string type_name, int type_value) = 0;
		virtual void write_member_value(std::string type_name, std::string type_value) = 0;
		virtual void write_member_value(std::string type_name, bool type_value) = 0;

		std::string file_name;
		bool is_read, is_wrote;
	};

	/* xml文件读入 */
	class xml_parser : public file_parser {
	public:
		xml_parser(std::string name) : file_parser(name) {}

		virtual void write_class_name(std::string class_name, int level, int size = -1) override {
			if (check_if_array(class_name, [&]() {
				if (size == 0) {
					out << "</" << end_arr_names.top() << ">";
					end_arr_names.pop();
				}
				else {
					out << std::endl;
				} })) {
				return;
			}
			out << "<" << class_name << ">" << std::endl;
		}

		virtual void write_member_name(std::string var_name, std::string class_name, int level) override {
			out << std::string(4 * (level), ' ') << "<" << var_name << ">";
			end_var_name = var_name;
			if (class_name.find('<') != std::string::npos && class_name.find('>') != std::string::npos) {
				end_arr_names.push(end_var_name);
			}
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

		virtual void read_file() override;

		virtual void write_file() override {
			// dic.clear(); /* 每一次读入时就清空，需要重新读入 */
			out.open(file_name, std::ios::out);
			out << "<?xml version=\"1.0\" encoding=\"utf - 8\" ?>" << std::endl;
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
			static mapping null_map = { "",0,false };
			return null_map; /* 需要返回一个值 */
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

		std::ifstream in;
		std::ofstream out;

		std::string end_var_name;
		std::stack<std::string> end_arr_names;
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

		virtual void read_file() override {}

		virtual void write_file() override {}

		virtual void write_file_finished() override {}

		mapping& operator[](std::string index) {
			static mapping null_map = { "",0,false };
			return null_map; /* 需要返回一个值 */
		}
	};
}