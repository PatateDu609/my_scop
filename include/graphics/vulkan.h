#ifndef SCOP_VULKAN_H
#define SCOP_VULKAN_H

#include <vulkan/vulkan.h>
#include <string>

namespace graphics {
	constexpr const char *ENGINE        = "gb_engine";
	constexpr uint32_t   ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);

	class VulkanInstance {
	public:
		VulkanInstance();
		~VulkanInstance();

	private:
		void create_instance();
		void create_debug_messenger();

		VkInstance               _instance;
		VkDebugUtilsMessengerEXT _debugMessenger;
	};
}

#endif //SCOP_VULKAN_H
