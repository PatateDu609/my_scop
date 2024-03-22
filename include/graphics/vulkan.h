#ifndef SCOP_VULKAN_H
#define SCOP_VULKAN_H

#include "pipeline.h"
#include "renderer.h"
#include "utils.h"

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
	void										create_short_lived_command_pool(const VkPhysicalDevice &physical);
	void										create_command_buffers();
	void										record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_idx, uint32_t frame_idx) const;
	void										create_sync_objects();
	void										create_vertex_buffer(const VkPhysicalDevice &physical);
	void										create_index_buffer(const VkPhysicalDevice &physical);
	void										create_uniform_buffers(const VkPhysicalDevice &physical);
	void										create_descriptor_pool();
	void										create_descriptor_sets();

	void										render(VkPhysicalDevice physical, uint32_t frame_idx) const;
	void										waitIdle() const;

	void										mark_framebuffer_resized();
	void										cleanup_swapchain();
	void										recreate_swapchain(VkPhysicalDevice physical);

private:
	std::pair<VkBuffer, VkDeviceMemory> create_buffer(const VkPhysicalDevice &physical, VkDeviceSize size, VkBufferUsageFlags usage,
													  VkMemoryPropertyFlags properties) const;
	static uint32_t						find_memory_type(const VkPhysicalDevice &physical, uint32_t type_filter, VkMemoryPropertyFlags properties);

	void								copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const;

	VkInstance							_instance{};
	VkDebugUtilsMessengerEXT			_debugMessenger{};
	std::shared_ptr<Renderer>			_renderer;

	VkSwapchainKHR						_swapchain{};
	std::vector<VkImage>				_swapchainImages;
	std::vector<VkImageView>			_swapchainImageViews;
	VkExtent2D							_swapchainExtent{};
	VkFormat							_swapchainFormat{};

	std::unique_ptr<Pipeline>			_pipeline{nullptr};
	std::vector<VkFramebuffer>			_framebuffers;
	bool								_framebufferResized{false};

	VkCommandPool						_commandPool{};
	std::vector<VkCommandBuffer>		_commandBuffers;

	VkCommandPool						_shortLivedCommandPool{};

	std::vector<VkSemaphore>			_imageAvailableSemaphores;
	std::vector<VkSemaphore>			_renderFinishedSemaphores;
	std::vector<VkFence>				_inFlightFences;

	VkDescriptorPool					_descriptorPool{};
	std::vector<VkDescriptorSet>		_descriptorSets;
	std::vector<VkBuffer>				_uniformBuffers;
	std::vector<VkDeviceMemory>			_uniformBuffersMemory;
	std::vector<void *>					_uniformBuffersMapped;

	VkDevice							_device{};

	std::vector<VertexData>				_vertices{};
	VkBuffer							_vertexBuffer{};
	VkDeviceMemory						_vertexBufferMemory{};

	std::vector<uint16_t>				_indices;
	VkBuffer							_indexBuffer{};
	VkDeviceMemory						_indexBufferMemory{};

	friend class Renderer;
};
} // namespace graphics

#endif // SCOP_VULKAN_H
