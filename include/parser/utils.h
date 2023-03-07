#ifndef SCOP_PARSER_UTILS_H
#define SCOP_PARSER_UTILS_H

#include <string>
#include <vector>

namespace parser {
	void split(std::vector<std::string> &args, const std::string &line, const std::string &delimiter = " ");
}

#endif //SCOP_PARSER_UTILS_H