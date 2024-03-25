#include "graphics/textures.h"

#include "stb_image.h"

namespace graphics::resources {

Texture Texture::load(const std::string &path) {
	if (path.empty()) {
		throw std::invalid_argument("couldn't load empty filename");
	}

	int		 w, h, channels;
	stbi_uc *img = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
	if (!img) {
		return {};
	}

	return {
		.pixels	  = std::shared_ptr<uint8_t>(img, stbi_image_free),
		.channels = 4UL,
		.w		  = static_cast<size_t>(w),
		.h		  = static_cast<size_t>(h),
	};
}

VkDeviceSize Texture::device_size() const {
	return w * h * channels;
}

Texture::operator bool() const {
	return pixels && w && h && channels;
}


} // namespace graphics::resources
