#define GLFW_INCLUDE_VULKAN

#include "graphics/renderer.h"

#include "application.h"
#include "graphics/vulkan.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

namespace graphics {

Renderer::Renderer(VulkanInstance *instance, GLFWwindow *window) : _instance(instance), _window(window), _surface() {
	init_surface();
}

void Renderer::init_surface() {
	if (glfwCreateWindowSurface(static_cast<VkInstance>(*_instance), _window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("couldn't create window surface");

	std::cerr << "Window surface created" << std::endl;
}

void Renderer::cleanup_surface() {
	vkDestroySurfaceKHR(static_cast<VkInstance>(*_instance), _surface, nullptr);
	_surface = nullptr;
}

VkSurfaceKHR Renderer::get_surface() const {
	return _surface;
}

GLFWwindow *Renderer::getWindow() const {
	return _window;
}

void Renderer::acquire_queues(const QueueFamilyIndices &indices) {
	if (indices.graphicsFamily)
		vkGetDeviceQueue(_instance->_device, indices.graphicsFamily.value(), 0, &_graphics);

	if (indices.presentFamily)
		vkGetDeviceQueue(_instance->_device, indices.presentFamily.value(), 0, &_present);
}

void Renderer::render(const uint32_t frame_idx) {
	const VkDevice		  &device				   = _instance->_device;
	const VkCommandBuffer &commandBuffer		   = _instance->_commandBuffers[frame_idx];
	const VkFence		  &inFlightFence		   = _instance->_inFlightFences[frame_idx];
	const VkSemaphore	  &imageAvailableSemaphore = _instance->_imageAvailableSemaphores[frame_idx];
	const VkSemaphore	  &renderFinishedSemaphore = _instance->_renderFinishedSemaphores[frame_idx];

	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t img_idx;
	vkAcquireNextImageKHR(device, _instance->_swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &img_idx);

	vkResetCommandBuffer(commandBuffer, 0);
	_instance->record_command_buffer(commandBuffer, img_idx);

	const std::array							  waitSemaphore{imageAvailableSemaphore};
	constexpr std::array<VkPipelineStageFlags, 1> waitPipelineStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	const std::array							  signalSemaphore{renderFinishedSemaphore};

	VkSubmitInfo								  submitInfo{};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount	= waitSemaphore.size();
	submitInfo.pWaitSemaphores		= waitSemaphore.data();
	submitInfo.pWaitDstStageMask	= waitPipelineStages.data();
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphore.size();
	submitInfo.pSignalSemaphores	= signalSemaphore.data();

	if (vkQueueSubmit(_graphics, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("couldn't submit graphics queue");
	}

	const std::array swapchains{_instance->_swapchain};
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = signalSemaphore.size();
	presentInfo.pWaitSemaphores	   = signalSemaphore.data();
	presentInfo.swapchainCount	   = swapchains.size();
	presentInfo.pSwapchains		   = swapchains.data();
	presentInfo.pImageIndices	   = &img_idx;
	presentInfo.pResults		   = nullptr;

	vkQueuePresentKHR(_present, &presentInfo);
}


} // namespace graphics
