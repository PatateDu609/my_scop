#include "graphics/utils.h"

#include "application.h"

#include <GLFW/glfw3.h>
#include <set>
#include <vulkan/vulkan.h>

std::vector<char const *> const graphics::VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

std::vector<char const *> const graphics::DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined __APPLE__ || (defined VK_KHR_portability_enumeration && VK_KHR_portability_enumeration == 1)
	"VK_KHR_portability_subset",
#endif
};

std::vector<char const *> graphics::get_required_extensions() {
	uint32_t				  glfwExtensionCount = 0;
	char const				**glfwExtensions	 = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<char const *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if constexpr (ENABLE_VALIDATION_LAYERS) // NOLINT: Simplify
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined __APPLE__ || (defined VK_KHR_portability_enumeration && VK_KHR_portability_enumeration == 1)
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

	return extensions;
}


bool graphics::check_validation_layer_support() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (auto const &layerName : VALIDATION_LAYERS) {
		bool found = false;
		for (auto const &layer : availableLayers) {
			if (std::strcmp(layer.layerName, layerName) == 0) {
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}
	return true;
}


bool graphics::check_device_extension_support(VkPhysicalDevice physicalDevice) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	for (auto const &extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}
