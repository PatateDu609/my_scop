#ifndef SCOP_RENDERER_H
#define SCOP_RENDERER_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "queue_families.h"

namespace graphics {
	class VulkanInstance;

	class Renderer {
	public:
		Renderer(VulkanInstance *instance, GLFWwindow *window);

		void cleanup_surface();
		[[nodiscard]] VkSurfaceKHR get_surface() const;

	private:
		void init_surface();
		void acquire_queues(const QueueFamilyIndices &indices);

		VulkanInstance *_instance;
		GLFWwindow     *_window;
		VkSurfaceKHR   _surface;
		VkQueue        _graphics;
		VkQueue        _present;

		friend class VulkanInstance;
	};
}

#endif //SCOP_RENDERER_H
