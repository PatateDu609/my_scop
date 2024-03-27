#define GLFW_INCLUDE_VULKAN
#define MATH_FORCE_DEPTH_ZERO_TO_ONE

#include "graphics/renderer.h"

#include "application.h"
#include "graphics/vulkan.h"
#include "maths/utils.h"

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

void Renderer::render(const VkPhysicalDevice physical, const uint32_t frame_idx) const {
	const VkDevice		  &device				   = _instance->_device;
	const VkCommandBuffer &commandBuffer		   = _instance->_commandBuffers[frame_idx];
	const VkFence		  &inFlightFence		   = _instance->_inFlightFences[frame_idx];
	const VkSemaphore	  &imageAvailableSemaphore = _instance->_imageAvailableSemaphores[frame_idx];
	const VkSemaphore	  &renderFinishedSemaphore = _instance->_renderFinishedSemaphores[frame_idx];

	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	uint32_t img_idx;
	{
		// ReSharper disable once CppTooWideScopeInitStatement
		const VkResult res = vkAcquireNextImageKHR(device, _instance->_swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &img_idx);

		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cerr << "SWAPCHAIN INVALID, RECREATING IT" << std::endl;
			_instance->recreate_swapchain(physical);
			return;
		}
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("couldn't acquire a new image from swapchain");
		}
	}

	vkResetFences(device, 1, &inFlightFence);
	vkResetCommandBuffer(commandBuffer, 0);
	_instance->record_command_buffer(commandBuffer, img_idx, frame_idx);

	updateUniformBuffer(frame_idx);

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


	{
		// ReSharper disable once CppTooWideScopeInitStatement
		const VkResult res = vkQueuePresentKHR(_present, &presentInfo);

		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || _instance->_framebufferResized) {
			std::cerr << "SWAPCHAIN INVALID, RECREATING IT" << std::endl;
			_instance->_framebufferResized = false;
			_instance->recreate_swapchain(physical);
		} else if (res != VK_SUCCESS) {
			throw std::runtime_error("couldn't present the resultant image to screen");
		}
	}
}

void Renderer::updateUniformBuffer(uint32_t frame_idx) const {
	const static auto	start_time	 = std::chrono::high_resolution_clock::now();

	const auto			current_time = std::chrono::high_resolution_clock::now();
	const float			elapsed		 = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
	const float			ratio		 = _instance->_swapchainExtent.width / static_cast<float>(_instance->_swapchainExtent.height);

	UniformBufferObject ubo{};
	ubo.model = maths::Mat4::rotate(elapsed * maths::rad(30), maths::Vec3(0, 0, 1));
	ubo.view  = maths::Mat4::lookAt(maths::Vec3(2.0f, 2.0f, 2.0f), maths::Vec3(0.0f, 0.0f, 0.0f), maths::Vec3(0.0f, 0.0f, 1.0f));
	ubo.proj  = maths::Mat4::perspective(maths::rad(45), ratio, 0.1f, 10.0f);

	if constexpr (DEBUG && false) {
		display_mat("model", ubo.model, 4, 4);
		display_mat("view", ubo.view, 4, 4);
		display_mat("proj", ubo.proj, 4, 4);
	}


	ubo.proj[1][1] *= -1;
	memcpy(_instance->_uniformBuffersMapped[frame_idx], &ubo, sizeof ubo);
}


} // namespace graphics
