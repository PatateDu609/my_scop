#include "application.h"

Application *app;


int main(int ac, char **av) {
	app = new Application(ac, av);

	return app->run();
}