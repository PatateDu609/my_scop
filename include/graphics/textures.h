#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

namespace graphics::resources {

struct Texture {
	std::shared_ptr<uint8_t> pixels;
	size_t					 w;
	size_t					 h;
	size_t					 channels;

	VkDeviceSize			 device_size() const;
	explicit				 operator bool() const;

	static Texture			 load(const std::string &path = "");
};

} // namespace graphics::resources


#endif // TEXTURES_HPP
