#include "application.h"

#include "graphics/queue_families.h"
#include "graphics/swap_chain.h"
#include "graphics/utils.h"
#include "parser/parser.h"

#include <iostream>
#include <map>
#include <sstream>

static void key_input(GLFWwindow *window, const int key, const int /*scancode*/, const int action, const int /*mods*/) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

static void framebufferResized(GLFWwindow *window, const int, const int) {
	const auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
	app->mark_framebuffer_resized();
}

Application::Application(const int ac, char **av) : _window(nullptr), _physicalDevice(VK_NULL_HANDLE) {
	if (ac != 2) {
		std::cerr << "usage: ./scop <scene file>" << std::endl;
		std::exit(1);
	}

	try {
		parser::parse(av[1]);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::exit(1);
	}
	init();
}


Application::~Application() {
	_instance.reset();

	_window.reset();
	glfwTerminate();
}


void Application::init() {
	init_window();
	_instance = std::make_unique<graphics::VulkanInstance>();
	_instance->set_renderer(_instance.get(), _window.get());
	select_physical_device();
	_instance->create_device(_physicalDevice);
	_instance->create_swapchain(_physicalDevice);
	_instance->create_image_views();
	_instance->create_pipeline("shaders/vertex.glsl", "shaders/frag.glsl");
	_instance->create_framebuffers();
	_instance->create_command_pool(_physicalDevice);
	_instance->create_short_lived_command_pool(_physicalDevice);
	_instance->create_texture_object(_physicalDevice, "resources/textures/texture.jpg");
	_instance->create_tex_img_view();
	_instance->create_tex_sampler(_physicalDevice);
	_instance->create_vertex_buffer(_physicalDevice);
	_instance->create_index_buffer(_physicalDevice);
	_instance->create_uniform_buffers(_physicalDevice);
	_instance->create_descriptor_pool();
	_instance->create_descriptor_sets();
	_instance->create_command_buffers();
	_instance->create_sync_objects();
}


void Application::init_window() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window.reset(glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr), glfwDestroyWindow);
	std::cerr << "GLFW window created" << std::endl;

	glfwSetWindowUserPointer(_window.get(), this);
	glfwSetKeyCallback(_window.get(), key_input);
	glfwSetFramebufferSizeCallback(_window.get(), framebufferResized);
}


int Application::run() const {
	uint64_t		   frame_cnt			  = 0;
	uint32_t		   frame_idx			  = 0;
	double			   time_since_last_update = glfwGetTime();

	std::ostringstream oss;

	while (!glfwWindowShouldClose(_window.get())) {
		glfwPollEvents();
		_instance->render(_physicalDevice, frame_idx);
		frame_cnt++;
		frame_idx = (frame_idx + 1) % MAX_FRAMES_IN_FLIGHT;

		if (glfwGetTime() - time_since_last_update >= 1.0) {
			oss.str("");
			oss << WINDOW_TITLE << " - FPS: " << frame_cnt;

			glfwSetWindowTitle(_window.get(), oss.str().c_str());
			time_since_last_update = glfwGetTime();
			frame_cnt			   = 0;
		}
	}

	_instance->waitIdle();

	return 0;
}

void Application::mark_framebuffer_resized() const {
	_instance->mark_framebuffer_resized();
}

void Application::select_physical_device() {
	auto													  physicalDevices = _instance->enumerate_physical_devices();
	std::multimap<uint32_t, VkPhysicalDevice, std::greater<>> ranking;

	for (const auto &physicalDevice : physicalDevices) {
		uint32_t score = check_physical_device_suitability(physicalDevice);
		if (score != 0) {
			ranking.emplace(score, physicalDevice);
			break;
		}
	}

	auto best = ranking.begin();
	if (best == ranking.end() || best->first == 0)
		throw std::runtime_error("couldn't find suitable device for Scop");

	_physicalDevice = best->second;
	std::cerr << "Successfully selected physical device" << std::endl;
}


uint32_t Application::check_physical_device_suitability(VkPhysicalDevice physicalDevice) const {
	VkPhysicalDeviceProperties deviceProperties{};
	VkPhysicalDeviceFeatures   deviceFeatures{};
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	if (!check_mandatory_features(physicalDevice, deviceProperties, deviceFeatures))
		return 0;

	uint32_t score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	score += deviceProperties.limits.maxImageDimension2D;
	score += static_cast<uint32_t>(deviceProperties.limits.maxSamplerAnisotropy);

	std::cerr << "Device `" << deviceProperties.deviceName << "` has score of " << score << std::endl;
	return score;
}


bool Application::check_mandatory_features(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures deviceFeatures) const {
	if (!deviceFeatures.sampleRateShading || !deviceFeatures.samplerAnisotropy)
		return false;

	auto surface			= _instance->get_surface();

	bool extensionSupported = graphics::check_device_extension_support(physicalDevice);
	auto queueFamilies		= graphics::find_queue_families(physicalDevice, surface);
	auto swapChainSupport	= graphics::query_swap_chain_support(physicalDevice, surface);

	return queueFamilies && extensionSupported && swapChainSupport;
}
