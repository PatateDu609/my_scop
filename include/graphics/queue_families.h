#ifndef SCOP_QUEUE_FAMILIES_H
#define SCOP_QUEUE_FAMILIES_H

#include <optional>
#include <set>
#include <vulkan/vulkan_core.h>

namespace graphics {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		explicit operator bool() const;
		explicit operator std::set<uint32_t>() const;
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
}

#endif //SCOP_QUEUE_FAMILIES_H
