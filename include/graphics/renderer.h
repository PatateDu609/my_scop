#ifndef SCOP_RENDERER_H
#define SCOP_RENDERER_H

#include "queue_families.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace graphics {
class VulkanInstance;

class Renderer {
public:
							   Renderer(VulkanInstance *instance, GLFWwindow *window);

	void					   cleanup_surface();
	[[nodiscard]] VkSurfaceKHR get_surface() const;

	GLFWwindow				  *getWindow() const;
	void					   render(VkPhysicalDevice physical, uint32_t frame_idx) const;

private:
	void			init_surface();
	void			acquire_queues(const QueueFamilyIndices &indices);

	VulkanInstance *_instance;
	GLFWwindow	   *_window;
	VkSurfaceKHR	_surface;
	VkQueue			_graphics{};
	VkQueue			_present{};

	friend class VulkanInstance;
};
} // namespace graphics

#endif // SCOP_RENDERER_H
