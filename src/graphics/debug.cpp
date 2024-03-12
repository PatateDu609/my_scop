#include "graphics/debug.h"

#include <iostream>
#include <sstream>
#include <string>


static std::string stringify_message_severity(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
	std::string severity;

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity = "[verbose]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity = "[inf]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = "[warn]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity = "[err]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		severity = "[fatal]";
		break;
	default:
		severity = "[unk]";
	}
	return severity;
}


static std::string stringify_message_type(VkDebugUtilsMessageTypeFlagsEXT messageType) {
	std::string type;

	switch (messageType) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		type = "general";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		type = "performance";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		type = "validation";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
		type = "device address binding";
		break;
	default:
		type = "unknown type";
	}
	return type;
}


VkBool32 graphics::debug::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT		 messageSeverity,
										 VkDebugUtilsMessageTypeFlagsEXT			 messageType,
										 VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData, void *) {
	//	if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
	//		return VK_FALSE;
	//	}

	std::string		   severity = stringify_message_severity(messageSeverity);
	std::string		   subject	= stringify_message_type(messageType);

	std::ostringstream oss;
	oss << "severity=\"" << severity << "\" subject=\"" << subject << "\" - " << pCallbackData->pMessage << "\n";

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ||
		messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ||
		messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT)
		std::cerr << oss.str();
	else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ||
			 messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
		std::cout << oss.str();
	else
		std::clog << oss.str();

	return VK_FALSE;
}


VkDebugUtilsMessengerCreateInfoEXT graphics::debug::get_debug_messenger_create_info() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
	createInfo.pfnUserCallback = debug_callback;
	createInfo.pUserData	   = nullptr;

	return createInfo;
}


VkResult graphics::debug::create_debug_utils_messenger_ext(VkInstance								 instance,
														   VkDebugUtilsMessengerCreateInfoEXT const *pCreateInfo,
														   VkAllocationCallbacks const				*pAllocator,
														   VkDebugUtilsMessengerEXT					*pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}


void graphics::debug::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
														VkAllocationCallbacks const *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
