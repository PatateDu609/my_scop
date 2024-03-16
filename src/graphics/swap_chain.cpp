#include "graphics/swap_chain.h"

#include <algorithm>

using graphics::SwapChainSupportDetails;

SwapChainSupportDetails graphics::query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


SwapChainSupportDetails::operator bool() const {
	return !formats.empty() && !presentModes.empty();
}

VkSurfaceFormatKHR SwapChainSupportDetails::chooseSwapSurfaceFormat() const {
	for (const auto &available : formats) {
		if (available.format == VK_FORMAT_B8G8R8A8_SRGB && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return available;
	}

	return formats[0];
}

VkPresentModeKHR SwapChainSupportDetails::chooseSwapPresentMode() const {
	for (const auto &available : presentModes) {
		if (available == VK_PRESENT_MODE_MAILBOX_KHR)
			return available;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainSupportDetails::chooseSwapExtent(GLFWwindow *window) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D extent{
		.width	= static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
	};

	extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}
