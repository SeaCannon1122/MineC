#include "testing.h"
#include <stdio.h>

#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"

#include <vulkan/vulkan.h>

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
        __debugbreak();\
    } \
} while(0)

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data
) {
	printf("Validation Error: %s\n", p_callback_data->pMessage);
	return false;
}

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	show_console_window();

	int window = window_create(100, 100, 700, 500, "window");

	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "testapp";
	app_info.pEngineName = "testengine";

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


	VkInstance instance;

	VKCall(vkCreateInstance(&instance_info, 0, &instance));

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	VkDebugUtilsMessengerEXT debug_messenger;

	if (vkCreateDebugUtilsMessengerEXT) {

		VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_info.pfnUserCallback = vulkan_debug_callbck;

		vkCreateDebugUtilsMessengerEXT(instance, &debug_info, 0, &debug_messenger);
	}


	VkSurfaceKHR surface;

	VKCall(create_vulkan_surface(instance, window, &surface));

	uint32_t gpu_count = 0;
	VkPhysicalDevice gpus[10];

	uint32_t graphics_idx = -1;

	VkPhysicalDevice gpu;

	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, 0));
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus));

	for (uint32_t i = 0; i < gpu_count; i++) {

		uint32_t queue_family_count = 0;

		VkQueueFamilyProperties queue_props[10];

		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, 0);
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, queue_props);

		uint32_t j = 0;
		for (; j < queue_family_count; j++) {

			if (queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				
				VkBool32 surface_support = VK_FALSE;
				VKCall(vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &surface_support));
				
				if (surface_support) {

					graphics_idx = j;
					gpu = gpus[i];
					break;
				}
					
			}

		}

		if (j != queue_family_count) break;

	}

	if (graphics_idx == -1) {
		printf("couldnt fint suitable device");
		goto close;
	}

	VkDevice device;

	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = graphics_idx;
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

	VKCall(vkCreateDevice(gpu, &device_info, 0, &device));



	VkSwapchainKHR swapchain;

	VkSwapchainCreateInfoKHR sc_info = { 0 };
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.surface = surface;

	VKCall(vkCreateSwapchainKHR(device, &sc_info, 0, &swapchain));



	while (!get_key_state(KEY_ESCAPE) && window_is_active(window)) {
		sleep_for_ms(100);
	}
	

close:
	window_destroy(window);

	networking_exit();
	platform_exit();

	return 0;
}