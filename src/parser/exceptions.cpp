#include <sstream>
#include "parser/parser.h"

parser::ifs_error::ifs_error(const std::string &filename) {
	std::ostringstream res;

	res << "an input file stream error happened: " << filename;
	err_str = res.str();
}


const char *parser::ifs_error::what() const {
	return err_str.c_str();
}