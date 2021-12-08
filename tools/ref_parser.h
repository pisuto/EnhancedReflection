#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

namespace ref {

	struct mapping {
		std::string value;
		int level;
		bool checked;
	};

	struct file_parser {
		file_parser(std::string name) : file_name(name), 
			is_read(false), is_wrote(false) {}
		
		bool is_read_file() const { return is_read; }
		bool is_write_file() const { return is_wrote; }

		virtual void read_file() = 0;
		virtual void write_file() = 0;
		virtual void write_file_finished() = 0;
		virtual void write_class_name(std::string, int size = -1) = 0;
		virtual void write_member_name(std::string, int) = 0;
		virtual void write_ident_space(int level) = 0;
		virtual void write_enter() = 0;
		virtual void write_class_end() = 0;
		virtual mapping& operator[](std::string) = 0;

		virtual void write_member_value(std::string type_name, int type_value) = 0;
		virtual void write_member_value(std::string type_name, std::string type_value) = 0;
		virtual void write_member_value(std::string type_name, bool type_value) = 0;

		std::string file_name;
		bool is_read, is_wrote;
	};

	class txt_parser : public file_parser {
	public:
		txt_parser(std::string name) : file_parser(name) {}

		virtual void write_class_name(std::string class_name, int size = -1) override {
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

		virtual void write_member_name(std::string var_name, int level) override {
			out << std::string(4 * (level), ' ') << var_name << " = ";
		}

		virtual void write_ident_space(int level) override {
			out << std::string(4 * (level), ' ');
		}

		virtual void write_enter() override {
			out << std::endl;
		}

		virtual void write_class_end() override {
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
}