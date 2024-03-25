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

struct VertexData {
	maths::Vec2												position;
	maths::Vec3												color;
	maths::Vec2												tex;

	static VkVertexInputBindingDescription					getBindingDesc();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescs();
};

struct UniformBufferObject {
	alignas(16) maths::Mat4 model{};
	alignas(16) maths::Mat4 view{};
	alignas(16) maths::Mat4 proj{};
};

} // namespace graphics

#endif // SCOP_UTILS_H
