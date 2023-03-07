#include "application.h"

int main(int ac, char **av) {
	Application app(ac, av);

	return app.run();
}