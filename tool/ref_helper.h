#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../reflection/ref_base.h"
#include "ref_parser.h"

namespace ref {

	struct type_descriptor;

	struct format_helper {
		format_helper(file_parser* method) : parser(method) {}
		
		~format_helper() { if (parser) delete parser; parser = nullptr; }
		
		template<typename ...Args>
		void read(Args&... args);

		template<typename T, typename ...Args>
		void read(T& data, Args&... args)
		{
			if (!parser->is_read_file()) {
				parser->read_file();
			}
			auto type_desc = ref::type_resolver<T>::get();
			type_desc->deserialize(parser, type_desc->full_name(), &data);
			read(args...);
		}

		template<>
		void read() {}

		template<typename ...Args>
		void write(Args&... args);

		template<typename T, typename ...Args>
		void write(T& data, Args&... args)
		{
			if (!parser->is_write_file()) {
				parser->write_file();
			}
			auto type_desc = ref::type_resolver<T>::get();
			type_desc->serialize(parser, &data);
			parser->write_next_type();
			write(args...);
		}

		template<>
		void write()
		{
			parser->write_file_finished();
		}

	private:
		file_parser* parser;
	};
}