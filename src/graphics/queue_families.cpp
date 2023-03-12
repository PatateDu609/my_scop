#include <vector>
#include "graphics/queue_families.h"

using graphics::QueueFamilyIndices;


QueueFamilyIndices::operator bool() const {
	return graphicsFamily && presentFamily;
}


QueueFamilyIndices::operator std::set<uint32_t>() const {
	std::set<uint32_t> result;

	if (graphicsFamily.has_value())
		result.insert(graphicsFamily.value());

	if (presentFamily.has_value())
		result.insert(presentFamily.value());

	return result;
}


QueueFamilyIndices graphics::find_queue_families(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	int      i = 0;
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (const auto &queueFamily: queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		if (indices)
			return indices;

		i++;
	}

	return indices;
}