#include <iostream>
#include "application.h"

Application *app;


int main(int ac, char **av) {
	try {
		app = new Application(ac, av);

		return app->run();
	} catch (const std::exception &e) {
		std::cerr << "unexpected exception caught: " << e.what() << std::endl;
		return 1;
	}
}