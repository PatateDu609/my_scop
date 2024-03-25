#include "graphics/vulkan.h"

#include "application.h"
#include "graphics/debug.h"
#include "graphics/queue_families.h"
#include "graphics/swap_chain.h"
#include "graphics/utils.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#define STR(x) #x

namespace graphics {

VulkanInstance::VulkanInstance() {
	_vertices = {
		VertexData{
				   .position = maths::Vec2{-0.5f, -0.5f},
				   .color	  = maths::Vec3{1.0f, 0.0f, 0.0f},
				   .tex	  = maths::Vec2{1.0f, 0.0f},
				   },
		VertexData{
				   .position = maths::Vec2{0.5f, -0.5f},
				   .color	  = maths::Vec3{0.0f, 1.0f, 0.0f},
				   .tex	  = maths::Vec2{0.0f, 0.0f},
				   },
		VertexData{
				   .position = maths::Vec2{0.5f, 0.5f},
				   .color	  = maths::Vec3{0.0f, 0.0f, 1.0f},
				   .tex	  = maths::Vec2{0.0f, 1.0f},
				   },
		VertexData{
				   .position = maths::Vec2{-0.5f, 0.5f},
				   .color	  = maths::Vec3{1.0f, 1.0f, 1.0f},
				   .tex	  = maths::Vec2{1.0f, 1.0f},
				   },
	};
	_indices = {0, 1, 2, 2, 3, 0};

	create_instance();
	create_debug_messenger();
}


VulkanInstance::~VulkanInstance() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
	}

	vkDestroyCommandPool(_device, _shortLivedCommandPool, nullptr);
	vkDestroyCommandPool(_device, _commandPool, nullptr);

	cleanup_swapchain();

	vkDestroySampler(_device, _sampler, nullptr);

	vkDestroyImageView(_device, _texImgView, nullptr);
	vkDestroyImage(_device, _texImg, nullptr);
	vkFreeMemory(_device, _texImgMemory, nullptr);

	vkDestroyBuffer(_device, _vertexBuffer, nullptr);
	vkFreeMemory(_device, _vertexBufferMemory, nullptr);

	vkDestroyBuffer(_device, _indexBuffer, nullptr);
	vkFreeMemory(_device, _indexBufferMemory, nullptr);

	for (size_t i = 0; i < _uniformBuffers.size(); i++) {
		vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
		vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
	}
	_uniformBuffersMapped.clear();

	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
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
		const auto ret = createImageView(_swapchainImages[i], _swapchainFormat);

		if (!ret) {
			throw std::runtime_error("couldn't create image view");
		}

		_swapchainImageViews[i] = *ret;
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
	_pipeline->create_descriptor_set_layout();
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

void VulkanInstance::create_short_lived_command_pool(const VkPhysicalDevice &physical) {
	const auto [graphicsQueueFamily, presentQueueFamily] = find_queue_families(physical, get_surface());

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = *graphicsQueueFamily;
	createInfo.flags			= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(_device, &createInfo, nullptr, &_shortLivedCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create short lived command pool for current device");
	}
	std::cerr << "Created successfully short lived command pool for current device" << std::endl;
}

void VulkanInstance::create_uniform_buffers(const VkPhysicalDevice &physical) {
	constexpr VkBufferUsageFlags	usage	   = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < _uniformBuffers.size(); i++) {
		constexpr VkDeviceSize bufferSize					   = sizeof(UniformBufferObject);

		std::tie(_uniformBuffers[i], _uniformBuffersMemory[i]) = create_buffer(physical, bufferSize, usage, properties);
		vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
		std::cerr << "Created successfully uniform buffer " << i << std::endl;
	}
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

void VulkanInstance::create_descriptor_pool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	poolSizes[1].type			 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes	 = poolSizes.data();
	createInfo.maxSets		 = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(_device, &createInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create new descriptor pool");
	}
	std::cerr << "Created successfully descriptor pool" << std::endl;
}

void VulkanInstance::create_descriptor_sets() {
	const std::vector			layouts(MAX_FRAMES_IN_FLIGHT, _pipeline->descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool		= _descriptorPool;
	allocateInfo.descriptorSetCount = layouts.size();
	allocateInfo.pSetLayouts		= layouts.data();

	_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(_device, &allocateInfo, _descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("couldn't allocate descriptor sets for current device");
	}
	std::cerr << "Allocated successfully descriptor sets for current device" << std::endl;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range  = VK_WHOLE_SIZE;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler	  = _sampler;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView	  = _texImgView;

		std::array<VkWriteDescriptorSet, 2> writeDescriptors{};
		writeDescriptors[0].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptors[0].dstSet			 = _descriptorSets[i];
		writeDescriptors[0].dstBinding		 = 0;
		writeDescriptors[0].dstArrayElement	 = 0;
		writeDescriptors[0].descriptorType	 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptors[0].descriptorCount	 = 1;
		writeDescriptors[0].pBufferInfo		 = &bufferInfo;
		writeDescriptors[0].pImageInfo		 = nullptr;
		writeDescriptors[0].pTexelBufferView = nullptr;

		writeDescriptors[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptors[1].dstSet			 = _descriptorSets[i];
		writeDescriptors[1].dstBinding		 = 1;
		writeDescriptors[1].dstArrayElement	 = 0;
		writeDescriptors[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptors[1].descriptorCount	 = 1;
		writeDescriptors[1].pBufferInfo		 = nullptr;
		writeDescriptors[1].pImageInfo		 = &imageInfo;
		writeDescriptors[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_device, writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		std::cerr << "Updated descriptor set number " << i << std::endl;
	}
}


void VulkanInstance::record_command_buffer(const VkCommandBuffer command_buffer, const uint32_t image_idx, uint32_t frame_idx) const {
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

	const std::array								   buffers{_vertexBuffer};
	constexpr std::array<VkDeviceSize, buffers.size()> offsets{0};
	vkCmdBindVertexBuffers(command_buffer, 0, buffers.size(), buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(command_buffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->layout, 0, 1, &_descriptorSets[frame_idx], 0, nullptr);
	vkCmdDrawIndexed(command_buffer, _indices.size(), 1, 0, 0, 0);

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

void VulkanInstance::create_vertex_buffer(const VkPhysicalDevice &physical) {
	const size_t   size = sizeof(_vertices[0]) * _vertices.size();

	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingMemory;
	{
		constexpr VkBufferUsageFlags	usage	   = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		std::tie(stagingBuffer, stagingMemory)	   = create_buffer(physical, size, usage, properties);
		std::cerr << "Created successfully staging buffer and allocated its memory (used for vertex buffer)" << std::endl;

		void *data;
		vkMapMemory(_device, stagingMemory, 0, size, 0, &data);
		memcpy(data, _vertices.data(), size);
		vkUnmapMemory(_device, stagingMemory);
	}

	{
		constexpr VkBufferUsageFlags	usage		 = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		constexpr VkMemoryPropertyFlags properties	 = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		std::tie(_vertexBuffer, _vertexBufferMemory) = create_buffer(physical, size, usage, properties);
		std::cerr << "Created successfully vertex buffer and allocated its memory" << std::endl;
	}

	copy_buffer(stagingBuffer, _vertexBuffer, size);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingMemory, nullptr);
}

void VulkanInstance::create_index_buffer(const VkPhysicalDevice &physical) {
	const size_t   size = sizeof(_indices[0]) * _indices.size();

	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingMemory;
	{
		constexpr VkBufferUsageFlags	usage	   = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		std::tie(stagingBuffer, stagingMemory)	   = create_buffer(physical, size, usage, properties);
		std::cerr << "Created successfully staging buffer and allocated its memory (used for index buffer)" << std::endl;

		void *data;
		vkMapMemory(_device, stagingMemory, 0, size, 0, &data);
		memcpy(data, _indices.data(), size);
		vkUnmapMemory(_device, stagingMemory);
	}

	{
		constexpr VkBufferUsageFlags	usage	   = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		std::tie(_indexBuffer, _indexBufferMemory) = create_buffer(physical, size, usage, properties);
		std::cerr << "Created successfully index buffer and allocated its memory" << std::endl;
	}

	copy_buffer(stagingBuffer, _indexBuffer, size);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingMemory, nullptr);
}

void VulkanInstance::create_texture_object(const VkPhysicalDevice &physical, std::string path) {
	_tex = resources::Texture::load(std::move(path));

	if (!_tex) {
		throw std::runtime_error("couldn't load image from file");
	}

	const VkDeviceSize				deviceSize = _tex.device_size();

	constexpr VkBufferUsageFlags	usage	   = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	auto [stagingBuffer, stagingMemory]		   = create_buffer(physical, deviceSize, usage, properties);

	void *data;
	vkMapMemory(_device, stagingMemory, 0, deviceSize, 0, &data);
	memcpy(data, _tex.pixels.get(), deviceSize);
	vkUnmapMemory(_device, stagingMemory);

	constexpr VkFormat				format	   = VK_FORMAT_R8G8B8A8_SRGB;
	constexpr VkImageTiling			tiling	   = VK_IMAGE_TILING_OPTIMAL;
	constexpr VkImageUsageFlags		imgUsage   = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	constexpr VkMemoryPropertyFlags props	   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	std::tie(_texImg, _texImgMemory)		   = createImage(physical, _tex.w, _tex.h, format, tiling, imgUsage, props);

	constexpr VkImageLayout oldLayout		   = VK_IMAGE_LAYOUT_UNDEFINED;
	constexpr VkImageLayout transitionalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	constexpr VkImageLayout finalLayout		   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	transition_image_layout(_texImg, format, oldLayout, transitionalLayout);
	copy_buffer_to_image(stagingBuffer, _texImg, _tex.w, _tex.h);
	transition_image_layout(_texImg, format, transitionalLayout, finalLayout);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingMemory, nullptr);
}

void VulkanInstance::create_tex_img_view() {
	const auto ret = createImageView(_texImg, VK_FORMAT_R8G8B8A8_SRGB);
	if (!ret) {
		throw std::runtime_error("couldn't create image view for main texture");
	}
	std::cerr << "Created successfully image view for main texture" << std::endl;
	_texImgView = *ret;
}

void VulkanInstance::create_tex_sampler(const VkPhysicalDevice &physical) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physical, &props);

	VkSamplerCreateInfo createInfo{};
	createInfo.sType				   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter			   = VK_FILTER_LINEAR;
	createInfo.minFilter			   = VK_FILTER_LINEAR;
	createInfo.addressModeU			   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV			   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW			   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable		   = VK_TRUE;
	createInfo.maxAnisotropy		   = props.limits.maxSamplerAnisotropy;
	createInfo.borderColor			   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable		   = VK_FALSE;
	createInfo.compareOp			   = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode			   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias			   = 0.0f;
	createInfo.minLod				   = 0.0f;
	createInfo.maxLod				   = 0.0f;

	if (vkCreateSampler(_device, &createInfo, nullptr, &_sampler) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create texture sampler");
	}
	std::cerr << "Created successfully texture sampler" << std::endl;
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

std::pair<VkBuffer, VkDeviceMemory> VulkanInstance::create_buffer(const VkPhysicalDevice &physical, const VkDeviceSize size, const VkBufferUsageFlags usage,
																  const VkMemoryPropertyFlags properties) const {
	VkBuffer		   buffer;
	VkDeviceMemory	   mem;

	VkBufferCreateInfo createInfo{};
	createInfo.sType	   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size		   = size;
	createInfo.usage	   = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(_device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create vertex buffer for current device");
	}

	VkMemoryRequirements memReqs{};
	vkGetBufferMemoryRequirements(_device, buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memReqs.size;
	allocInfo.memoryTypeIndex = find_memory_type(physical, memReqs.memoryTypeBits, properties);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &mem) != VK_SUCCESS) {
		throw std::runtime_error("couldn't allocate memory for vertex buffer");
	}
	vkBindBufferMemory(_device, buffer, mem, 0);

	return {buffer, mem};
}

std::pair<VkImage, VkDeviceMemory> VulkanInstance::createImage(const VkPhysicalDevice physical, const size_t w, const size_t h, const VkFormat format,
															   const VkImageTiling tiling, const VkImageUsageFlags usage,
															   const VkMemoryPropertyFlags props) const {
	VkImage			  img;
	VkDeviceMemory	  imgMem;

	VkImageCreateInfo createInfo{};
	createInfo.sType		 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType	 = VK_IMAGE_TYPE_2D;
	createInfo.extent.width	 = w;
	createInfo.extent.height = h;
	createInfo.extent.depth	 = 1;
	createInfo.mipLevels	 = 1;
	createInfo.arrayLayers	 = 1;
	createInfo.format		 = format;
	createInfo.tiling		 = tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage		 = usage;
	createInfo.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples		 = VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags		 = 0;

	if (vkCreateImage(_device, &createInfo, nullptr, &img) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create image");
	}
	std::cerr << "Created successfully image" << std::endl;

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(_device, img, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType			  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memReqs.size;
	allocInfo.memoryTypeIndex = find_memory_type(physical, memReqs.memoryTypeBits, props);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &imgMem) != VK_SUCCESS) {
		throw std::runtime_error("couldn't allocate memory for image");
	}
	std::cerr << "Allocated successfully memory for image" << std::endl;

	vkBindImageMemory(_device, img, imgMem, 0);

	return {img, imgMem};
}

std::optional<VkImageView> VulkanInstance::createImageView(const VkImage image, const VkFormat format) const {
	VkImageViewCreateInfo createInfo{};
	createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image						   = image;
	createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format						   = format;
	createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel   = 0;
	createInfo.subresourceRange.levelCount	   = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount	   = 1;

	VkImageView view;
	if (vkCreateImageView(_device, &createInfo, nullptr, &view) != VK_SUCCESS) {
		return std::nullopt;
	}

	return view;
}

uint32_t VulkanInstance::find_memory_type(const VkPhysicalDevice &physical, const uint32_t type_filter, const VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physical, &memProps);

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("couldn't find suitable memory type");
}

VkCommandBuffer VulkanInstance::beginSingleTimeCommand() const {
	VkCommandBuffer				cmdBuffer;
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool		= _shortLivedCommandPool;
	allocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_device, &allocateInfo, &cmdBuffer) != VK_SUCCESS) {
		throw std::runtime_error("couldn't allocate new command buffer for current device");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	return cmdBuffer;
}

void VulkanInstance::endSingleTimeCommand(const VkCommandBuffer cmdBuffer) const {
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &cmdBuffer;

	vkQueueSubmit(_renderer->_graphics, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_renderer->_graphics);

	vkFreeCommandBuffers(_device, _shortLivedCommandPool, 1, &cmdBuffer);
}

void VulkanInstance::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const {
	const VkCommandBuffer cmdBuffer = beginSingleTimeCommand();

	VkBufferCopy		  cpy{};
	cpy.size	  = size;
	cpy.srcOffset = 0;
	cpy.dstOffset = 0;
	vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &cpy);

	endSingleTimeCommand(cmdBuffer);
}

void VulkanInstance::transition_image_layout(VkImage image, VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout) const {
	const VkCommandBuffer cmdBuffer = beginSingleTimeCommand();

	VkImageMemoryBarrier  barrier{};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= oldLayout;
	barrier.newLayout						= newLayout;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	VkPipelineStageFlagBits srcStage;
	decltype(srcStage)		dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage			  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage			  = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		srcStage			  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage			  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition");
	}
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	// clang-format off
	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	// clang-format on

	endSingleTimeCommand(cmdBuffer);
}

void VulkanInstance::copy_buffer_to_image(const VkBuffer buffer, const VkImage image, const uint32_t w, const uint32_t h) const {
	const VkCommandBuffer cmdBuffer = beginSingleTimeCommand();

	VkBufferImageCopy	  region{};
	region.bufferOffset					   = 0;
	region.bufferRowLength				   = 0;
	region.bufferImageHeight			   = 0;

	region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel	   = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount	   = 1;

	region.imageOffset					   = {0, 0, 0};
	region.imageExtent					   = {w, h, 1};

	vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommand(cmdBuffer);
}


} // namespace graphics
