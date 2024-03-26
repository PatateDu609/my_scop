#include "graphics/pipeline.h"

#include "graphics/utils.h"

#include <iostream>
#include <memory>
#include <sstream>

namespace graphics {

Pipeline::Pipeline(VkDevice &device, std::string vertex_path, std::string fragment_path) : device(device) {
	shaders.emplace("vertex", ShaderData(std::make_shared<resources::Shader>(std::move(vertex_path))));
	shaders.emplace("fragment", ShaderData(std::make_shared<resources::Shader>(std::move(fragment_path))));
}

Pipeline::~Pipeline() {
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyPipeline(device, pipeline, nullptr);

	vkDestroyPipelineLayout(device, layout, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto &[stage_name, data] : shaders) {
		if (!data.module) {
			continue;
		}
		vkDestroyShaderModule(device, *data.module, nullptr);
	}
}

auto Pipeline::compile_shaders() const -> bool {
	bool res = true;

	for (const auto &[stage_name, data] : shaders) {
		res &= data.resource->compile();
	}

	return res;
}

auto Pipeline::get_num_errors() const -> std::pair<size_t, size_t> {
	std::pair<size_t, size_t> ret;
	auto &[ret_errs, ret_warns] = ret;

	for (const auto &[stage_name, dat] : shaders) {
		const auto &[errs, warns]  = dat.resource->get_num_errors();

		ret_errs				  += errs;
		ret_warns				  += warns;
	}

	return ret;
}

auto Pipeline::errors() const -> std::string {
	std::ostringstream oss;

	for (const auto &[stage_name, data] : shaders) {
		oss << stage_name << " shader: " << data.resource->errors() << "\n";
	}
	oss << std::flush;
	return oss.str();
}

auto Pipeline::create_module(const VkDevice &device, const std::vector<uint32_t> &shader, const std::string &stage) -> VkShaderModule {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shader.size() * sizeof(*shader.data());
	createInfo.pCode	= shader.data();

	VkShaderModule ret;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &ret) != VK_SUCCESS) {
		throw std::runtime_error("couldn't generate shader module from given code in " + stage + " stage");
	}

	std::cerr << "Created successfully shader module for " << stage << " stage" << std::endl;
	return ret;
}

void Pipeline::setup_shader_modules() {
	for (auto &[stage_name, data] : shaders) {
		if (data.module) {
			continue;
		}

		data.module = create_module(device, data.resource->compiled(), stage_name);

		decltype(data.stage)::value_type stage{};
		stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

		if (stage_name == "vertex") {
			stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		} else if (stage_name == "fragment") {
			stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		} else {
			throw std::runtime_error("stage " + stage_name + " not handled yet");
		}

		stage.module = *data.module;
		stage.pName	 = "main";

		data.stage	 = stage;
	}
}

void Pipeline::setup_render_pass(const VkFormat &format, const VkFormat &depthFormat) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format		   = format;
	colorAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout	   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format		   = depthFormat;
	depthAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout	   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // idx of layout in fragment shader
	colorAttachmentRef.layout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1; // idx of layout in fragment shader
	depthAttachmentRef.layout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass	= VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass	= 0;
	subpassDependency.srcStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	const std::array	   attachments{colorAttachment, depthAttachment};

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments	 = attachments.data();
	renderPassCreateInfo.subpassCount	 = 1;
	renderPassCreateInfo.pSubpasses		 = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies	 = &subpassDependency;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create render pass for current device");
	}
	std::cerr << "Created successfully render pass for current device" << std::endl;
}

void Pipeline::create_descriptor_set_layout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding			= 0;
	uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount	= 1;
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding			  = 1;
	samplerBinding.descriptorType	  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount	  = 1;
	samplerBinding.stageFlags		  = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.pImmutableSamplers = nullptr;

	const std::array				bindings{uboLayoutBinding, samplerBinding};

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings	= bindings.data();
	createInfo.flags		= 0;

	if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create descriptor set layout");
	}
	std::cerr << "Created descriptor set layout successfully" << std::endl;
}


void Pipeline::setup(const VkExtent2D &extent) {
	static std::vector				 dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType					 = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount		 = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates			 = dynamicStates.data();

	const auto							&bindingDesc = VertexData::getBindingDesc();
	const auto							&attrsDescs	 = VertexData::getAttributeDescs();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType							  = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount	  = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions	  = &bindingDesc;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = attrsDescs.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions	  = attrsDescs.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	inputAssemblyCreateInfo.sType				   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
	inputAssemblyCreateInfo.topology			   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport viewport{};
	viewport.x		  = 0.0F;
	viewport.y		  = 0.0F;
	viewport.height	  = static_cast<float>(extent.height);
	viewport.width	  = static_cast<float>(extent.width);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 0.0F;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo{};
	viewportCreateInfo.sType		 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.scissorCount	 = 1;
	viewportCreateInfo.viewportCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType					 = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable		 = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth				 = 1.0F;
	rasterizerCreateInfo.cullMode				 = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace				 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable		 = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0F;
	rasterizerCreateInfo.depthBiasClamp			 = 0.0F;
	rasterizerCreateInfo.depthBiasSlopeFactor	 = 0.0F;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable	= VK_FALSE;
	multisampleCreateInfo.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.minSampleShading		= 1.0F;
	multisampleCreateInfo.pSampleMask			= nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable		= VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendingAttachment{};
	colorBlendingAttachment.colorWriteMask =
		static_cast<uint32_t>(VK_COLOR_COMPONENT_R_BIT) | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendingAttachment.blendEnable			= VK_TRUE;
	colorBlendingAttachment.blendEnable			= VK_FALSE;
	colorBlendingAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendingAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendingAttachment.colorBlendOp		= VK_BLEND_OP_ADD;
	colorBlendingAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendingAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendingAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
	colorBlendingCreateInfo.sType			  = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable	  = VK_FALSE;
	colorBlendingCreateInfo.logicOp			  = VK_LOGIC_OP_COPY;
	colorBlendingCreateInfo.attachmentCount	  = 1;
	colorBlendingCreateInfo.pAttachments	  = &colorBlendingAttachment;
	colorBlendingCreateInfo.blendConstants[0] = 0.0F;
	colorBlendingCreateInfo.blendConstants[1] = 0.0F;
	colorBlendingCreateInfo.blendConstants[2] = 0.0F;
	colorBlendingCreateInfo.blendConstants[3] = 0.0F;

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType				   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable	   = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable	   = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp		   = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds		   = 0.0f; // Optional
	depthStencilCreateInfo.maxDepthBounds		   = 1.0f; // Optional
	depthStencilCreateInfo.stencilTestEnable	   = VK_FALSE;
	depthStencilCreateInfo.front				   = {}; // Optional
	depthStencilCreateInfo.back				   = {}; // Optional

	VkPipelineLayoutCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineCreateInfo.setLayoutCount		  = 1;
	pipelineCreateInfo.pSetLayouts			  = &descriptorSetLayout;
	pipelineCreateInfo.pushConstantRangeCount = 0;
	pipelineCreateInfo.pPushConstantRanges	  = nullptr;

	if (vkCreatePipelineLayout(device, &pipelineCreateInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create pipeline layout for current device");
	}

	std::cerr << "Created successfully pipeline layout for current device" << std::endl;

	std::vector<VkPipelineShaderStageCreateInfo> stages;
	for (const auto &[stageName, data] : shaders) {
		stages.push_back(*data.stage);
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType			   = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount		   = stages.size();
	graphicsPipelineCreateInfo.pStages			   = stages.data();
	graphicsPipelineCreateInfo.pVertexInputState   = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState	   = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState   = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState  = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState	   = &colorBlendingCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState	   = &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.layout			   = layout;
	graphicsPipelineCreateInfo.renderPass		   = renderPass;
	graphicsPipelineCreateInfo.subpass			   = 0;
	graphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex   = -1;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create graphics pipeline for current device");
	}

	std::cerr << "Created successfully a graphics pipeline for the current device" << std::endl;
}

} // namespace graphics
