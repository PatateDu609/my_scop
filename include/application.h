#ifndef SCOP_APPLICATION_H
#define SCOP_APPLICATION_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "graphics/vulkan.h"

#ifndef DEBUG
#define DEBUG true
#endif

constexpr const char *WINDOW_TITLE            = "scop";
constexpr uint32_t   APPLICATION_VERSION      = VK_MAKE_VERSION(1, 0, 0);
constexpr uint32_t   WINDOW_WIDTH             = 800;
constexpr uint32_t   WINDOW_HEIGHT            = 600;
constexpr bool       ENABLE_VALIDATION_LAYERS = DEBUG;

class Application {
public:
	Application() = delete;
	Application(const Application &) = delete;
	Application &operator=(const Application &) = delete;

	Application(int ac, char **av);
	~Application();

	int run();

private:
	void init();

	void init_window();

	void select_physical_device();
	uint32_t check_physical_device_suitability(VkPhysicalDevice physicalDevice) const;
	bool check_mandatory_features(VkPhysicalDevice physicalDevice,
	                              VkPhysicalDeviceProperties deviceProperties,
	                              VkPhysicalDeviceFeatures deviceFeatures) const;

	GLFWwindow               *_window;
	graphics::VulkanInstance *_instance;
	VkPhysicalDevice         _physicalDevice;
};

extern Application *app;

#endif //SCOP_APPLICATION_H
