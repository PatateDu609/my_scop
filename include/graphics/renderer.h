#ifndef SCOP_RENDERER_H
#define SCOP_RENDERER_H

#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

namespace graphics {
	class VulkanInstance;

	class Renderer {
	public:
		Renderer(VulkanInstance *instance, GLFWwindow *window);

		void cleanup_surface();
		VkSurfaceKHR get_surface() const;

	private:
		void init_surface();

		VulkanInstance *_instance;
		GLFWwindow     *_window;
		VkSurfaceKHR   _surface;
	};
}

#endif //SCOP_RENDERER_H
