#ifndef _XRCE_PARSER_H_
#define _XRCE_PARSER_H_

#include "Serializer.h"

#include <functional>

class XRCEParser
{
public:
	XRCEParser(char* buffer, size_t size, std::function<void()> create_callback, std::function< void() > write_data_callback, std::function< void() > read_data_callback)
		: deserializer_(buffer, size),
		create_callback_(create_callback),
		write_data_callback_(write_data_callback),
		read_data_callback_(read_data_callback)
	{

	}
	bool parse();
private:

	bool process_create();
	bool process_write_data();
	bool process_read_data();

	std::function< void() > create_callback_;
	std::function< void() > write_data_callback_;
	std::function< void() > read_data_callback_;
	Serializer deserializer_;
};

#endif // !_XRCE_PARSER_H_
