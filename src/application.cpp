#include <iostream>
#include "application.h"
#include "parser/parser.h"

Application::Application(int ac, char **av) {
	if (ac != 2) {
		std::cerr << "usage: ./scop <scene file>" << std::endl;
		std::exit(1);
	}

	try {
		parser::parse(av[1]);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		std::exit(1);
	}
}


void Application::run() {

}