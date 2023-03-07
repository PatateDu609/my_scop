#include "parser/utils.h"


void parser::split(std::vector<std::string> &args, const std::string &line, const std::string &delimiter) {
	size_t      pos_start = 0;
	size_t      pos_end;
	size_t      delim_len = delimiter.length();
	std::string token;

	args.clear();
	while ((pos_end = line.find(delimiter, pos_start)) != std::string::npos) {
		token     = line.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		args.push_back(token);
	}

	args.push_back(line.substr(pos_start));
}
