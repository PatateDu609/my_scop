#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>
#include "graphics/utils.h"
#include "application.h"

extern const std::vector<const char *> graphics::VALIDATION_LAYERS = {
		"VK_LAYER_KHRONOS_validation"
};


std::vector<const char *> graphics::get_required_extensions() {
	uint32_t   glfwExtensionCount = 0;
	const char **glfwExtensions   = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (ENABLE_VALIDATION_LAYERS) //NOLINT: Simplify
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}


bool graphics::check_validation_layer_support() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto &layerName: VALIDATION_LAYERS) {
		bool            found = false;
		for (const auto &layer: availableLayers) {
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