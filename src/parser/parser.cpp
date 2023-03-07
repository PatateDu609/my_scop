#include <fstream>
#include "parser/parser.h"
#include "parser/utils.h"


parser::File parser::file;


void parser::parse(const std::string &filename) {
	std::ifstream ifs(filename);

	if (!ifs)
		throw parser::ifs_error(filename);

	file.vertices.clear();

	std::string              line;
	std::vector<std::string> args;
	while (std::getline(ifs, line)) {
		split(args, line);
		std::string id = args[0];

		args.erase(args.begin());
		if (id == "v")
			file.vertices.emplace_back(args);
		else if (id == "vt")
			file.texture_coordinates.emplace_back(args);
		else if (id == "f")
			file.faces.emplace_back(args);
	}

	file.faces.shrink_to_fit();
	file.vertices.shrink_to_fit();
	file.texture_coordinates.shrink_to_fit();

	ifs.close();
}