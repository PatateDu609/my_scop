#include "graphics/pipeline.h"

#include <iostream>
#include <memory>
#include <sstream>

namespace graphics {

Pipeline::Pipeline(VkDevice &device, std::string vertex_path, std::string fragment_path) : device(device) {
	shaders.emplace("vertex", ShaderData(std::make_shared<resources::Shader>(std::move(vertex_path))));
	shaders.emplace("fragment", ShaderData(std::make_shared<resources::Shader>(std::move(fragment_path))));
}

Pipeline::~Pipeline() {
	if (pipeline)
		vkDestroyPipeline(device, pipeline, nullptr);
	if (layout)
		vkDestroyPipelineLayout(device, layout, nullptr);
	if (renderPass)
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

void Pipeline::setup_render_pass(const VkFormat &format) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format		   = format;
	colorAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout	   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // idx of layout in fragment shader
	colorAttachmentRef.layout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments	 = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments	 = &colorAttachment;
	renderPassCreateInfo.subpassCount	 = 1;
	renderPassCreateInfo.pSubpasses		 = &subpass;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("couldn't create render pass for current device");
	}
	std::cerr << "Created successfully render pass for current device" << std::endl;
}

void Pipeline::setup(const VkExtent2D &extent) {
	static std::vector				 dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType			 = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates	 = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType							  = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount	  = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions	  = nullptr; // Optional
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions	  = nullptr; // Optional

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
	rasterizerCreateInfo.frontFace				 = VK_FRONT_FACE_CLOCKWISE;
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

	VkPipelineLayoutCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineCreateInfo.setLayoutCount		  = 0;
	pipelineCreateInfo.pSetLayouts			  = nullptr;
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
	graphicsPipelineCreateInfo.pDepthStencilState  = nullptr;
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