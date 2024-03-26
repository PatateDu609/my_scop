#ifndef SCOP_PIPELINE_H
#define SCOP_PIPELINE_H

#include "graphics/shaders.h"

#include <string>
#include <vulkan/vulkan_core.h>

namespace graphics {

class Pipeline {
public:
		 Pipeline(VkDevice &device, std::string vertex_path, std::string fragment_path);
	~	 Pipeline();

		 Pipeline(Pipeline &&other)				   = default;
	auto operator=(Pipeline &&other) -> Pipeline & = default;

private:
	static auto create_module(const VkDevice &device, const std::vector<uint32_t> &shader, const std::string &stage) -> VkShaderModule;
	struct ShaderData {
		std::optional<VkShaderModule>				   module;
		std::optional<VkPipelineShaderStageCreateInfo> stage;
		std::shared_ptr<resources::Shader>			   resource;

		explicit									   ShaderData(std::shared_ptr<resources::Shader> res) : resource(std::move(res)) {
		}
	};


public:
	[[nodiscard]] auto compile_shaders() const -> bool;
	[[nodiscard]] auto get_num_errors() const -> std::pair<size_t, size_t>;
	[[nodiscard]] auto errors() const -> std::string;

	void			   setup_shader_modules();
	void			   setup_render_pass(const VkFormat &format, const VkFormat &depthFormat);
	void			   create_descriptor_set_layout();
	void			   setup(const VkExtent2D &extent);

private:
	std::unordered_map<std::string, ShaderData> shaders;

	std::reference_wrapper<VkDevice>			device;
	VkRenderPass								renderPass{};
	VkDescriptorSetLayout						descriptorSetLayout{};
	VkPipelineLayout							layout{};
	VkPipeline									pipeline{};

public:
		 Pipeline()								   = delete;
		 Pipeline(const Pipeline &)				   = delete;
	auto operator=(const Pipeline &) -> Pipeline & = delete;

	friend class VulkanInstance;
};

} // namespace graphics

#endif // SCOP_PIPELINE_H
