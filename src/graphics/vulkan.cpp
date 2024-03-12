#include <memory>
#include <stdexcept>
#include <iostream>
#include "graphics/vulkan.h"
#include "application.h"
#include "graphics/utils.h"
#include "graphics/debug.h"
#include "graphics/queue_families.h"
#include <utility>


using graphics::VulkanInstance;


VulkanInstance::VulkanInstance() : _instance(), _debugMessenger(), _renderer(), _device() {
	create_instance();
	create_debug_messenger();
}


VulkanInstance::~VulkanInstance() {
	vkDestroyDevice(_device, nullptr);

	if constexpr (ENABLE_VALIDATION_LAYERS) //NOLINT: Simplify
		debug::destroy_debug_utils_messenger_ext(_instance, _debugMessenger, nullptr);

	_renderer->cleanup_surface();

	vkDestroyInstance(_instance, nullptr);
}


VkSurfaceKHR VulkanInstance::get_surface() const {
	return _renderer->get_surface();
}


void VulkanInstance::create_instance() {
	if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) //NOLINT: Simplify
		throw std::runtime_error("validation layers requested but not supported");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pApplicationName = WINDOW_TITLE;
	appInfo.applicationVersion = APPLICATION_VERSION;
	appInfo.pEngineName = ENGINE;
	appInfo.engineVersion = ENGINE_VERSION;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = get_required_extensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	createInfo.enabledLayerCount = 0;
	if (ENABLE_VALIDATION_LAYERS) { //NOLINT: Simplify
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

		auto debugCreateInfo = debug::get_debug_messenger_create_info();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
	}

	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		throw std::runtime_error("couldn't create vulkan instance");

	std::cerr << "Vulkan instance created" << std::endl;
}


void VulkanInstance::create_debug_messenger() {
	if constexpr (!ENABLE_VALIDATION_LAYERS) // NOLINT
		return;

	auto createInfo = debug::get_debug_messenger_create_info();

	if (debug::create_debug_utils_messenger_ext(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("couldn't setup debug messenger");
	std::cerr << "Debug messenger is set up" << std::endl;
}


VulkanInstance::operator VkInstance() {
	return _instance;
}


std::vector<VkPhysicalDevice> VulkanInstance::enumerate_physical_devices() const {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevice(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevice.data());

	return physicalDevice;
}


void VulkanInstance::create_device(VkPhysicalDevice device) {
	auto indices = graphics::find_queue_families(device, _renderer->get_surface());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	auto uniqueQueueFamilies = static_cast<std::set<uint32_t>>(indices);
	float queuePriority = 1.0f;

	for (uint32_t queueFamily: uniqueQueueFamilies) {
		decltype(queueCreateInfos)::value_type queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
	createInfo.enabledLayerCount = 0;

	if (ENABLE_VALIDATION_LAYERS) { //NOLINT: Simplify
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}

	if (vkCreateDevice(device, &createInfo, nullptr, &_device) != VK_SUCCESS)
		throw std::runtime_error("failed to create vulkan logical device");

	_renderer->acquire_queues(indices);

	std::cerr << "Created successfully a logical device and acquired graphics and present queues" << std::endl;
}
