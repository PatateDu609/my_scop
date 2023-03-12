#ifndef SCOP_SWAP_CHAIN_H
#define SCOP_SWAP_CHAIN_H

#include <vulkan/vulkan.h>
#include <vector>

namespace graphics {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR        capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>   presentModes;

		explicit operator bool() const;
	};

	SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif //SCOP_SWAP_CHAIN_H
