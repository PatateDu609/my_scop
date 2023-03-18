#ifndef SCOP_VULKAN_H
#define SCOP_VULKAN_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "renderer.h"

namespace graphics {
	constexpr const char *ENGINE        = "gb_engine";
	constexpr uint32_t   ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);

	class VulkanInstance {
	public:
		VulkanInstance();
		~VulkanInstance();

		void set_renderer(Renderer *renderer);
		[[nodiscard]] VkSurfaceKHR get_surface() const;

		[[nodiscard]] std::vector<VkPhysicalDevice> enumerate_physical_devices() const;
		void create_device(VkPhysicalDevice device);

	private:
		void create_instance();
		void create_debug_messenger();

		operator VkInstance(); // NOLINT(google-explicit-constructor)

		VkInstance               _instance;
		VkDebugUtilsMessengerEXT _debugMessenger;
		Renderer                 *_renderer;

		VkDevice _device;

		friend class Renderer;
	};
}

#endif //SCOP_VULKAN_H
