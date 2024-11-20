#include "general/platformlib/platform/platform.h"

#include <stdio.h>
#include <malloc.h>

#include "rendering_context.h"

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) return result;\
} while(0)

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT * p_callback_data,
	void* p_user_data
) {
	printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
	return 0;
}

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger) {

	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	app_info.pEngineName = engine_name;

	char* instance_extensions[] = {
		PLATFORM_VK_SURFACE_EXTENSION,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	char* layers[] = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkInstanceCreateInfo instance_info = { 0 };
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.ppEnabledExtensionNames = instance_extensions;
	instance_info.enabledExtensionCount = 3;
	instance_info.ppEnabledLayerNames = layers;
	instance_info.enabledLayerCount = 1;

	VKCall(vkCreateInstance(&instance_info, 0, instance));

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
	
	if (vkCreateDebugUtilsMessengerEXT) {

		VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_info.pfnUserCallback = vulkan_debug_callbck;

		vkCreateDebugUtilsMessengerEXT(*instance, &debug_info, 0, debug_messenger);
	}

	return VK_SUCCESS;
}

VkResult get_first_suitable_VkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_index) {

	uint32_t gpu_count = 0;
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, 0));
	VkPhysicalDevice* gpus = alloca(gpu_count * sizeof(VkPhysicalDevice));
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus));

	for (uint32_t i = 0; i < gpu_count; i++) {

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, 0);
		VkQueueFamilyProperties* queue_props = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, queue_props);

		uint32_t j = 0;
		for (; j < queue_family_count; j++) {

			if (queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {

				VkBool32 surface_support = VK_FALSE;
				VKCall(vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &surface_support));

				if (surface_support) {

					*queue_index = j;
					*gpu = gpus[i];
					
					return VK_SUCCESS;
				}

			}

		}

	}

	return 187187187;
}

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice gpu, uint32_t queue_idx, float queue_priority, VkDevice* device) {

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = queue_idx;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;

	char* device_extentions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo device_info = { 0 };
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.queueCreateInfoCount = 1;
	device_info.ppEnabledExtensionNames = device_extentions;
	device_info.enabledExtensionCount = 1;

	VKCall(vkCreateDevice(gpu, &device_info, 0, device));

	return VK_SUCCESS;
}