#ifndef SCOP_UTILS_H
#define SCOP_UTILS_H

#include <vector>

namespace graphics {
	extern const std::vector<const char *> VALIDATION_LAYERS;
	extern const std::vector<const char *> DEVICE_EXTENSIONS;

	std::vector<const char *> get_required_extensions();
	bool check_validation_layer_support();
	bool check_device_extension_support(VkPhysicalDevice physicalDevice);
}

#endif //SCOP_UTILS_H
