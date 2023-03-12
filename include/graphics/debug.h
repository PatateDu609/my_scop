#ifndef SCOP_DEBUG_H
#define SCOP_DEBUG_H

#include <vulkan/vulkan.h>

namespace graphics::debug {
	VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *userData
	);

	VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info();


	VkResult
	create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	                                 const VkAllocationCallbacks *pAllocator,
	                                 VkDebugUtilsMessengerEXT *pDebugMessenger);

	void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	                                       const VkAllocationCallbacks *pAllocator);
}

#endif //SCOP_DEBUG_H
