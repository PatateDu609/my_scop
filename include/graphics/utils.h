#ifndef SCOP_UTILS_H
#define SCOP_UTILS_H

#include "maths/mat.h"
#include "maths/vec.h"

#include <vector>
#include <vulkan/vulkan_core.h>

namespace graphics {
extern const std::vector<const char *> VALIDATION_LAYERS;
extern const std::vector<const char *> DEVICE_EXTENSIONS;

std::vector<const char *>			   get_required_extensions();
bool								   check_validation_layer_support();
bool								   check_device_extension_support(VkPhysicalDevice physicalDevice);
VkSampleCountFlagBits				   get_max_usable_sample_count(const VkPhysicalDevice &physical);

struct VertexData {
	bool operator==(const VertexData &rhs) const {
		return std::tie(position, color, tex) == std::tie(rhs.position, rhs.color, rhs.tex);
	}

	bool operator!=(const VertexData &rhs) const {
		return !(*this == rhs);
	}

	static VkVertexInputBindingDescription					getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();

	maths::Vec3												position;
	maths::Vec3												color;
	maths::Vec2												tex;
};

struct UniformBufferObject {
	alignas(16) maths::Mat4 model{};
	alignas(16) maths::Mat4 view{};
	alignas(16) maths::Mat4 proj{};
};

} // namespace graphics

template <>
struct std::hash<graphics::VertexData> {
	std::size_t operator()(const graphics::VertexData &vertex_data) const noexcept {
		return std::hash<maths::Vec3>{}(vertex_data.position) ^ std::hash<maths::Vec3>{}(vertex_data.color) ^ std::hash<maths::Vec2>{}(vertex_data.tex);
	}
};

#endif // SCOP_UTILS_H
