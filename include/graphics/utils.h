#ifndef SCOP_UTILS_H
#define SCOP_UTILS_H

#include <vector>

namespace graphics {
	extern const std::vector<const char *> VALIDATION_LAYERS;

	std::vector<const char *> get_required_extensions();
	bool check_validation_layer_support();
}

#endif //SCOP_UTILS_H
