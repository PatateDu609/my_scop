#include <iostream>
#include <map>
#include "application.h"
#include "parser/parser.h"
#include "graphics/utils.h"
#include "graphics/queue_families.h"
#include "graphics/swap_chain.h"


Application::Application(int ac, char **av) : _window(nullptr), _instance(), _physicalDevice(VK_NULL_HANDLE) {
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
	_instance->set_renderer(new graphics::Renderer(_instance, _window));
	select_physical_device();
	_instance->create_device(_physicalDevice);
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


void Application::select_physical_device() {
	auto                                                       physicalDevices = _instance->enumerate_physical_devices();
	std::multimap<uint32_t, VkPhysicalDevice, std::greater<> > ranking;

	for (const auto &physicalDevice: physicalDevices) {
		uint32_t score = check_physical_device_suitability(physicalDevice);
		if (score != 0) {
			ranking.emplace(score, physicalDevice);
			break;
		}
	}

	auto best = ranking.begin();
	if (best == ranking.end() || best->first == 0)
		throw std::runtime_error("couldn't find suitable device for Scop");

	_physicalDevice = best->second;
	std::cerr << "Successfully selected physical device" << std::endl;
}


uint32_t Application::check_physical_device_suitability(VkPhysicalDevice physicalDevice) const {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures   deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	if (!check_mandatory_features(physicalDevice, deviceProperties, deviceFeatures))
		return 0;

	uint32_t score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	score += deviceProperties.limits.maxImageDimension2D;
	score += static_cast<uint32_t>(deviceProperties.limits.maxSamplerAnisotropy);

	std::cerr << "Device `" << deviceProperties.deviceName << "` has score of " << score << std::endl;
	return score;
}


bool Application::check_mandatory_features(VkPhysicalDevice physicalDevice,
                                           VkPhysicalDeviceProperties,
                                           VkPhysicalDeviceFeatures deviceFeatures) const {
	if (!deviceFeatures.geometryShader || !deviceFeatures.sampleRateShading || !deviceFeatures.samplerAnisotropy)
		return false;

	auto surface = _instance->get_surface();

	bool extensionSupported = graphics::check_device_extension_support(physicalDevice);
	auto queueFamilies      = graphics::find_queue_families(physicalDevice, surface);
	auto swapChainSupport   = graphics::query_swap_chain_support(physicalDevice, surface);

	return queueFamilies && extensionSupported && swapChainSupport;
}