#include <iostream>
#include "application.h"

int main(int ac, char **av) {
	try {
		auto app = std::make_shared<Application>(ac, av);

		return app->run();
	} catch (const std::exception &e) {
		std::cerr << "unexpected exception caught: " << e.what() << std::endl;
		return 1;
	}
}
