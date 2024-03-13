#ifndef SCOP_SWAP_CHAIN_H
#define SCOP_SWAP_CHAIN_H

#include "GLFW/glfw3.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace graphics {
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR		 capabilities;
	std::vector<VkSurfaceFormatKHR>	 formats;
	std::vector<VkPresentModeKHR>	 presentModes;

	[[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat() const;
	[[nodiscard]] VkPresentModeKHR	 chooseSwapPresentMode() const;
	[[nodiscard]] VkExtent2D		 chooseSwapExtent(GLFWwindow *window) const;

	explicit						 operator bool() const;
};

SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
} // namespace graphics

#endif // SCOP_SWAP_CHAIN_H
