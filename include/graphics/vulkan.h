#ifndef SCOP_VULKAN_H
#define SCOP_VULKAN_H

#include "pipeline.h"
#include "renderer.h"

#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

namespace graphics {

constexpr auto	   ENGINE		  = "gb_engine";
constexpr uint32_t ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);

class VulkanInstance {
public:
	 VulkanInstance();
	~VulkanInstance();

private:
	void	 create_instance();
	void	 create_debug_messenger();
	explicit operator VkInstance() const;

public:
	template <typename... Args>
	void set_renderer(Args &&...args) {
		_renderer = std::make_shared<Renderer>(std::forward<Args>(args)...);
	}

	[[nodiscard]] VkSurfaceKHR					get_surface() const;

	[[nodiscard]] std::vector<VkPhysicalDevice> enumerate_physical_devices() const;

	void										create_device(VkPhysicalDevice device);
	void										create_swapchain(VkPhysicalDevice physical);
	void										create_image_views();
	void										create_pipeline(std::string vertex_shader, std::string fragment_shader);
	void										create_framebuffers();
	void										create_command_pool(const VkPhysicalDevice &physical);
	void										create_command_buffers();
	void										record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_idx) const;
	void										create_sync_objects();

	void										render(VkPhysicalDevice physical, uint32_t frame_idx) const;
	void										waitIdle() const;

	void										mark_framebuffer_resized();
	void										cleanup_swapchain();
	void										recreate_swapchain(VkPhysicalDevice physical);

private:
	VkInstance					 _instance{};
	VkDebugUtilsMessengerEXT	 _debugMessenger{};
	std::shared_ptr<Renderer>	 _renderer;

	VkSwapchainKHR				 _swapchain{};
	std::vector<VkImage>		 _swapchainImages;
	std::vector<VkImageView>	 _swapchainImageViews;
	VkExtent2D					 _swapchainExtent{};
	VkFormat					 _swapchainFormat{};

	std::unique_ptr<Pipeline>	 _pipeline{nullptr};
	std::vector<VkFramebuffer>	 _framebuffers;
	bool						 _framebufferResized{false};

	VkCommandPool				 _commandPool;
	std::vector<VkCommandBuffer> _commandBuffers;

	std::vector<VkSemaphore>	 _imageAvailableSemaphores;
	std::vector<VkSemaphore>	 _renderFinishedSemaphores;
	std::vector<VkFence>		 _inFlightFences;

	VkDevice					 _device{};

	friend class Renderer;
};
} // namespace graphics

#endif // SCOP_VULKAN_H
