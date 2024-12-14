#include "graphics.h"

#include "client/game_client.h"
#include "general/rendering/rendering_window.h"
#include <malloc.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data
) {
	printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
	return 0;
}

uint32_t list_gpus(struct game_client* game) {

	struct rendering_window_state rws;
	rendering_window_get_data(game->graphics_state.dummy_window, &rws);

	uint32_t gpu_count = 0;
	VKCall(vkEnumeratePhysicalDevices(game->graphics_state.instance, &gpu_count, 0));
	VKCall(vkEnumeratePhysicalDevices(game->graphics_state.instance, &gpu_count, game->graphics_state.gpus));

	for (uint32_t i = 0; i < gpu_count; i++) {

		VkPhysicalDeviceProperties dev_prop;
		vkGetPhysicalDeviceProperties(game->graphics_state.gpus[i], &dev_prop);

		memcpy(game->application_state.machine_info.gpus[i].name, dev_prop.deviceName, 256);
		game->application_state.machine_info.gpus[i].usable = 0;
		game->application_state.machine_info.gpus[i].gpu_type = dev_prop.deviceType;
		game->application_state.machine_info.gpus[i].vulkan_version_major = VK_API_VERSION_MAJOR(dev_prop.apiVersion);
		game->application_state.machine_info.gpus[i].vulkan_version_minor = VK_API_VERSION_MINOR(dev_prop.apiVersion);
		game->application_state.machine_info.gpus[i].vulkan_version_patch = VK_API_VERSION_PATCH(dev_prop.apiVersion);

		uint32_t format_count = 0;
		VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(game->graphics_state.gpus[i], rws.surface, &format_count, 0));
		VkSurfaceFormatKHR* surface_formats = alloca(format_count * sizeof(VkSurfaceFormatKHR));
		VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(game->graphics_state.gpus[i], rws.surface, &format_count, surface_formats));

		uint32_t found_format = 0;
		for (uint32_t k = 0; k < format_count; k++) if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
			game->graphics_state.gpu_surface_formats[i] = surface_formats[k];
			found_format = 1;
			break;
		}
		if (found_format == 0) continue;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(game->graphics_state.gpus[i], &queue_family_count, 0);
		VkQueueFamilyProperties* queue_props = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(game->graphics_state.gpus[i], &queue_family_count, queue_props);

		for (uint32_t j = 0; j < queue_family_count; j++) {

			if ((queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_props[j].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT)) {

				VkBool32 surface_support = VK_FALSE;
				VKCall(vkGetPhysicalDeviceSurfaceSupportKHR(game->graphics_state.gpus[i], j, rws.surface, &surface_support));

				if (surface_support) {
					game->graphics_state.gpu_queue_indices[i] = j;
					game->application_state.machine_info.gpus[i].usable = 1;
					
				}

			}

		}

	}

	game->application_state.machine_info.gpu_count = gpu_count;

	return 0;

}

uint32_t graphics_create(struct game_client* game) {

	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "blocks2";
	app_info.pEngineName = "blocks2_angine";

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

	VKCall(vkCreateInstance(&instance_info, 0, &game->graphics_state.instance));

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = vkGetInstanceProcAddr(game->graphics_state.instance, "vkCreateDebugUtilsMessengerEXT");

	game->graphics_state.debug_messenger = 0;
	if (vkCreateDebugUtilsMessengerEXT) {

		VkDebugUtilsMessengerCreateInfoEXT debug_info = { 0 };
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_info.pfnUserCallback = vulkan_debug_callbck;

		vkCreateDebugUtilsMessengerEXT(game->graphics_state.instance, &debug_info, 0, &game->graphics_state.debug_messenger);
	}

	game->graphics_state.dummy_window = window_create(50, 50, 50, 50, "a", 0);
	rendering_window_new(game->graphics_state.dummy_window, game->graphics_state.instance);

	rendering_window_new(game->application_state.window, game->graphics_state.instance);

	list_gpus(game);

	game->graphics_state.device = 0;
	for (int i = 0; i < game->application_state.machine_info.gpu_count; i++) if (game->application_state.machine_info.gpus[i].usable) {

		game->application_state.machine_info.gpu_index = i;

		graphics_device_create(game, i);
		break;
	}

	if (game->graphics_state.device == 0) {
		graphics_destroy(game);
		return 1;
	}

	return 0;
}

uint32_t graphics_destroy(struct game_client* game) {

	if (game->graphics_state.device != 0) graphics_device_destroy(game);

	rendering_window_destroy(game->graphics_state.dummy_window);
	rendering_window_destroy(game->application_state.window);

	window_destroy(game->graphics_state.dummy_window);

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = vkGetInstanceProcAddr(game->graphics_state.instance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessengerEXT != 0) {
		vkDestroyDebugUtilsMessengerEXT(game->graphics_state.instance, game->graphics_state.debug_messenger, 0);
	}

	vkDestroyInstance(game->graphics_state.instance, 0);

	return 0;
}