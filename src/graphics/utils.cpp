#include "graphics/utils.h"

#include "application.h"

#include <GLFW/glfw3.h>
#include <set>
#include <vulkan/vulkan.h>

namespace graphics {

const std::vector<const char *> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined __APPLE__ || (defined VK_KHR_portability_enumeration && VK_KHR_portability_enumeration == 1)
	"VK_KHR_portability_subset",
#endif
};

std::vector<const char *> get_required_extensions() {
	uint32_t				  glfwExtensionCount = 0;
	const char				**glfwExtensions	 = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if constexpr (ENABLE_VALIDATION_LAYERS) // NOLINT: Simplify
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined __APPLE__ || (defined VK_KHR_portability_enumeration && VK_KHR_portability_enumeration == 1)
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

	return extensions;
}


bool check_validation_layer_support() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto &layerName : VALIDATION_LAYERS) {
		bool found = false;
		for (const auto &layer : availableLayers) {
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


bool check_device_extension_support(const VkPhysicalDevice physicalDevice) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	for (const auto &extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

VkVertexInputBindingDescription VertexData::getBindingDesc() {
	VkVertexInputBindingDescription res{};
	res.binding	  = 0;
	res.stride	  = sizeof(VertexData);
	res.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return res;
}

std::array<VkVertexInputAttributeDescription, 3> VertexData::getAttributeDescs() {
	std::array<VkVertexInputAttributeDescription, 3> attrs{};

	attrs[0].binding  = 0;
	attrs[0].location = 0;
	attrs[0].format	  = VK_FORMAT_R32G32_SFLOAT;
	attrs[0].offset	  = offsetof(VertexData, position);

	attrs[1].binding  = 0;
	attrs[1].location = 1;
	attrs[1].format	  = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset	  = offsetof(VertexData, color);

	attrs[2].binding = 0;
	attrs[2].location = 2;
	attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[2].offset = offsetof(VertexData, tex);

	return attrs;
}


} // namespace graphics
