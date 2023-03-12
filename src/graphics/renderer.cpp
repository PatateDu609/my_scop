#define GLFW_INCLUDE_VULKAN

#include <glfw/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "graphics/renderer.h"
#include "graphics/vulkan.h"


using graphics::Renderer;


Renderer::Renderer(VulkanInstance *instance, GLFWwindow *window) : _instance(instance), _window(window), _surface() {
	init_surface();
}


void Renderer::init_surface() {
	if (glfwCreateWindowSurface(*_instance, _window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("couldn't create window surface");

	std::cerr << "Window surface created" << std::endl;
}


void Renderer::cleanup_surface() {
	vkDestroySurfaceKHR(*_instance, _surface, nullptr);
	_surface = nullptr;
}


VkSurfaceKHR Renderer::get_surface() const {
	return _surface;
}