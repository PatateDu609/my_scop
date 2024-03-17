#include "graphics/vulkan.h"

#include "application.h"
#include "graphics/debug.h"
#include "graphics/queue_families.h"
#include "graphics/swap_chain.h"
#include "graphics/utils.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace graphics {

VulkanInstance::VulkanInstance() {
	create_instance();
	create_debug_messenger();
}


VulkanInstance::~VulkanInstance() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
	}

	vkDestroyCommandPool(_device, _commandPool, nullptr);

	cleanup_swapchain();

	_pipeline.reset();
	vkDestroyDevice(_device, nullptr);

	if constexpr (ENABLE_VALIDATION_LAYERS) // NOLINT: Simplify
		debug::destroy_debug_utils_messenger_ext(_instance, _debugMessenger, nullptr);

	_renderer->cleanup_surface();

	vkDestroyInstance(_instance, nullptr);
}


VkSurfaceKHR VulkanInstance::get_surface() const {
	return _renderer->get_surface();
}


void VulkanInstance::create_instance() {
	if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) // NOLINT: Simplify
		throw std::runtime_error("validation layers requested but not supported");

	VkApplicationInfo appInfo{};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion		   = VK_API_VERSION_1_0;
	appInfo.pApplicationName   = WINDOW_TITLE;
	appInfo.applicationVersion = APPLICATION_VERSION;
	appInfo.pEngineName		   = ENGINE;
	appInfo.engineVersion	   = ENGINE_VERSION;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType				   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo		   = &appInfo;

	const auto extensions			   = get_required_extensions();
	createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

#if defined __APPLE__ || (defined VK_KHR_portability_enumeration && VK_KHR_portability_enumeration == 1)
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	createInfo.enabledLayerCount = 0;
	if (ENABLE_VALIDATION_LAYERS) { // NOLINT: Simplify
		createInfo.enabledLayerCount   = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

		auto debugCreateInfo		   = debug::get_debug_messenger_create_info();
		createInfo.pNext			   = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	}

	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		throw std::runtime_error("couldn't create vulkan instance");

	std::cerr << "Vulkan instance created" << std::endl;
}


void VulkanInstance::create_debug_messenger() {
	if constexpr (!ENABLE_VALIDATION_LAYERS)
		return;

	const auto createInfo = debug::get_debug_messenger_create_info();

	if (debug::create_debug_utils_messenger_ext(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("couldn't setup debug messenger");
	std::cerr << "Debug messenger is set up" << std::endl;
}


VulkanInstance::operator VkInstance() const {
	return _instance;
}


std::vector<VkPhysicalDevice> VulkanInstance::enumerate_physical_devices() const {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevice(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevice.data());

	return physicalDevice;
}


void VulkanInstance::create_device(VkPhysicalDevice device) {
	auto								 indices = graphics::find_queue_families(device, _renderer->get_surface());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	auto								 uniqueQueueFamilies = static_cast<std::set<uint32_t>>(indices);
	float								 queuePriority		 = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		decltype(queueCreateInfos)::value_type queueCreateInfo{};
		queueCreateInfo.sType			 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount		 = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType				   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos	   = queueCreateInfos.data();
	createInfo.queueCreateInfoCount	   = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures		   = &deviceFeatures;
	createInfo.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
	createInfo.enabledLayerCount	   = 0;

	if (ENABLE_VALIDATION_LAYERS) { // NOLINT: Simplify
		createInfo.enabledLayerCount   = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}

	if (vkCreateDevice(device, &createInfo, nullptr, &_device) != VK_SUCCESS)
		throw std::runtime_error("failed to create vulkan logical device");

	_renderer->acquire_queues(indices);

	std::cerr << "Created successfully a logical device and acquired graphics and present queues" << std::endl;
}

void VulkanInstance::create_swapchain(const VkPhysicalDevice physical) {
	const SwapChainSupportDetails swapChainSupport = query_swap_chain_support(physical, get_surface());

	const VkPresentModeKHR		  presentMode	   = swapChainSupport.chooseSwapPresentMode();
	const auto [formatKHR, colorSpace]			   = swapChainSupport.chooseSwapSurfaceFormat();
	const VkExtent2D extent						   = swapChainSupport.chooseSwapExtent(_renderer->_window);

	uint32_t		 imageCount					   = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType									 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface									 = get_surface();

	createInfo.minImageCount							 = imageCount;
	createInfo.imageExtent								 = extent;
	createInfo.imageFormat								 = formatKHR;
	createInfo.imageColorSpace							 = colorSpace;
	createInfo.imageArrayLayers							 = 1;
	createInfo.imageUsage								 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const auto [graphicsQueueFamily, presentQueueFamily] = find_queue_families(physical, get_surface());
	const std::array indices_arr{graphicsQueueFamily.value(), presentQueueFamily.value()};

	if (graphicsQueueFamily != presentQueueFamily) {
		createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = indices_arr.size();
		createInfo.pQueueFamilyIndices	 = indices_arr.data();
	} else {
		createInfo.imageSharingMode		 = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices	 = nullptr;
	}

	createInfo.preTransform	  = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode	  = presentMode;
	createInfo.clipped		  = VK_TRUE;

	createInfo.oldSwapchain	  = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create swapchain");
	}

	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

	_swapchainFormat = formatKHR;
	_swapchainExtent = extent;

	std::cerr << "Successfully created swapchain and retrieved associated images" << std::endl;
}

void VulkanInstance::cleanup_swapchain() {
	for (const auto &framebuffer : _framebuffers) {
		vkDestroyFramebuffer(_device, framebuffer, nullptr);
	}
	_framebuffers.clear();

	for (const auto &imageView : _swapchainImageViews) {
		vkDestroyImageView(_device, imageView, nullptr);
	}

	_swapchainImageViews.clear();

	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	_swapchainImages.clear();
	_swapchain = VK_NULL_HANDLE;
}


void VulkanInstance::recreate_swapchain(const VkPhysicalDevice physical) {
	{
		int w, h;
		do {
			glfwGetFramebufferSize(_renderer->_window, &w, &h);
			glfwWaitEvents();
		} while (w == 0 || h == 0);
	}

	vkDeviceWaitIdle(_device);

	cleanup_swapchain();

	create_swapchain(physical);
	create_image_views();
	create_framebuffers();
}


void VulkanInstance::create_image_views() {
	_swapchainImageViews.resize(_swapchainImages.size());

	for (size_t i = 0; i < _swapchainImageViews.size(); i++) {
		VkImageViewCreateInfo createInfo{};

		createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image						   = _swapchainImages[i];
		createInfo.format						   = _swapchainFormat;
		createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_2D;

		createInfo.components.a					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b					   = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount	   = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount	   = 1;

		if (vkCreateImageView(_device, &createInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create image view");
		}

		std::cerr << "Successfully created image view no " << i << std::endl;
	}
}

void VulkanInstance::create_pipeline(std::string vertex_shader, std::string fragment_shader) {
	_pipeline = std::make_unique<Pipeline>(_device, std::move(vertex_shader), std::move(fragment_shader));

	if (!_pipeline->compile_shaders()) {
		const auto &[errors, warnings] = _pipeline->get_num_errors();
		std::cerr << "got " << errors << " errors and " << warnings << " warnings\n" << _pipeline->errors() << std::endl;
		throw std::runtime_error("couldn't compile shaders");
	}
	_pipeline->setup_shader_modules();

	_pipeline->setup_render_pass(_swapchainFormat);
	_pipeline->setup(_swapchainExtent);
}

void VulkanInstance::create_framebuffers() {
	_framebuffers.resize(_swapchainImageViews.size());

	for (size_t i = 0; i < _framebuffers.size(); i++) {
		std::array				attachments = {_swapchainImageViews[i]};

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType		   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass	   = _pipeline->renderPass;
		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments	   = attachments.data();
		createInfo.width		   = _swapchainExtent.width;
		createInfo.height		   = _swapchainExtent.height;
		createInfo.layers		   = 1;


		std::ostringstream oss;
		oss << "framebuffer[" << i << "] for current device";

		if (vkCreateFramebuffer(_device, &createInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create " + oss.str());
		}

		std::cerr << "Created successfully " << oss.str() << "\n";
	}
	std::cerr << std::flush;
}

void VulkanInstance::create_command_pool(const VkPhysicalDevice &physical) {
	const auto [graphicsQueueFamily, presentQueueFamily] = find_queue_families(physical, get_surface());

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = *graphicsQueueFamily;
	createInfo.flags			= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(_device, &createInfo, nullptr, &_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create command pool for current device");
	}
	std::cerr << "Created successfully command pool for current device" << std::endl;
}

void VulkanInstance::create_command_buffers() {
	_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool		= _commandPool;
	allocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

	if (vkAllocateCommandBuffers(_device, &allocateInfo, _commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("couldn't allocate command buffer for current device");
	}
	std::cerr << "Created successfully command buffer for current device" << std::endl;
}

void VulkanInstance::record_command_buffer(const VkCommandBuffer command_buffer, const uint32_t image_idx) const {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType			   = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags			   = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("couldn't begin command buffer");
	}

	constexpr VkClearValue clearValue{.color = {.float32 = {0.0F, 0.0F, 0.0F, 1.0F}}};
	VkRenderPassBeginInfo  renderPassInfo{};
	renderPassInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass		 = _pipeline->renderPass;
	renderPassInfo.framebuffer		 = _framebuffers[image_idx];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _swapchainExtent;
	renderPassInfo.clearValueCount	 = 1;
	renderPassInfo.pClearValues		 = &clearValue;

	vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipeline);

	VkViewport viewport{};
	viewport.x		  = 0.0F;
	viewport.y		  = 0.0F;
	viewport.width	  = static_cast<float>(_swapchainExtent.width);
	viewport.height	  = static_cast<float>(_swapchainExtent.height);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 1.0F;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _swapchainExtent;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdDraw(command_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("couldn't record command buffer");
	}
}

void VulkanInstance::create_sync_objects() {
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("couldn't create semaphore[" + std::to_string(i) + "] for device");
		}
		if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &_inFlightFences[i])) {
			throw std::runtime_error("couldn't create fence[" + std::to_string(i) + "] for device");
		}
	}

	std::cerr << "Created successfully all fences and semaphores needed" << std::endl;
}

void VulkanInstance::render(const VkPhysicalDevice physical, const uint32_t frame_idx) const {
	_renderer->render(physical, frame_idx);
}

void VulkanInstance::waitIdle() const {
	vkDeviceWaitIdle(_device);
}

void VulkanInstance::mark_framebuffer_resized() {
	_framebufferResized = true;
}


} // namespace graphics
