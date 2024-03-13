#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
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


void Renderer::acquire_queues(const QueueFamilyIndices &indices) {
	if (indices.graphicsFamily)
		vkGetDeviceQueue(_instance->_device, indices.graphicsFamily.value(), 0, &_graphics);

	if (indices.presentFamily)
		vkGetDeviceQueue(_instance->_device, indices.presentFamily.value(), 0, &_present);
}
