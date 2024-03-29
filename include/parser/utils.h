#ifndef SCOP_PARSER_UTILS_H
#define SCOP_PARSER_UTILS_H

#include <string>
#include <vector>

namespace parser {
	using std::operator ""sv;
	void split(std::vector<std::string> &args, const std::string_view &line, const std::string_view &delimiter = " "sv);
}

#endif //SCOP_PARSER_UTILS_H
