#include <iostream>
#include "application.h"
#include "parser/parser.h"


Application::Application(int ac, char **av) : _window(nullptr), _instance() {
	if (ac != 2) {
		std::cerr << "usage: ./scop <scene file>" << std::endl;
		std::exit(1);
	}

	try {
		parser::parse(av[1]);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::exit(1);
	}
	init();
}


Application::~Application() {
	delete _instance;

	glfwDestroyWindow(_window);
	glfwTerminate();
}


void Application::init() {
	init_window();
	_instance = new graphics::VulkanInstance();
}


void Application::init_window() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	std::cerr << "GLFW window created" << std::endl;

	glfwSetWindowUserPointer(_window, this);
}


int Application::run() {
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();
	}

	return 0;
}