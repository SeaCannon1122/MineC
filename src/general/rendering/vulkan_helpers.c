#include <stdio.h>
#include <malloc.h>

#include "vulkan_helpers.h"

void* load_file(uint8_t* filename, uint32_t* size) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	uint8_t* buffer = (uint8_t*)malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(uint8_t), fileSize, file);

	fclose(file);

	*size = fileSize;

	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callbck(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT msg_flags,
	const VkDebugUtilsMessengerCallbackDataEXT * p_callback_data,
	void* p_user_data
) {
	printf("--------------------------------------------------------------------------\n\nValidation Error: %s\n\n\n\n", p_callback_data->pMessage);
	return 0;
}

uint32_t get_vulkan_version(uint32_t* major, uint32_t* minor, uint32_t* patch) {

	uint32_t apiVersion;
	VKCall(vkEnumerateInstanceVersion(&apiVersion));
	
	*major = VK_API_VERSION_MAJOR(apiVersion);
	*minor = VK_API_VERSION_MINOR(apiVersion);
	*patch = VK_API_VERSION_PATCH(apiVersion);

	return 0;
}

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger) {

	VkApplicationInfo app_info = { 0 };
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	app_info.pEngineName = engine_name;
	app_info.apiVersion = VK_API_VERSION_1_3;

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

VkResult destroy_VkInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger) {
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessengerEXT != 0) {
		vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, 0);
	}

	vkDestroyInstance(instance, 0);

	return VK_SUCCESS;
}

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device) {

	uint32_t gpu_count = 0;
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, 0));
	VkPhysicalDevice* gpus = alloca(gpu_count * sizeof(VkPhysicalDevice));
	VKCall(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus));


	VkSurfaceKHR surface;
	uint32_t window = window_create(0, 0, 1, 1, " ", 0);
	VKCall(create_vulkan_surface(instance, window, &surface));

	for (uint32_t i = 2; i < gpu_count; i++) {

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, 0);
		VkQueueFamilyProperties* queue_props = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_family_count, queue_props);

		uint32_t j = 0;
		for (; j < queue_family_count; j++) {

			if ((queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_props[j].queueFlags & VK_QUEUE_TRANSFER_BIT) && (queue_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT)) {

				VkBool32 surface_support = VK_FALSE;
				VKCall(vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &surface_support));

				if (surface_support) {

					*queue_family_index = j;
					*gpu = gpus[i];

					VKCall(destroy_vulkan_surface(instance, surface));
					window_destroy(window);

					goto gpu_found;
				}

			}

		}

	}

	VKCall(destroy_vulkan_surface(instance, surface));
	window_destroy(window);

	return 187187187;

gpu_found:

	VkDeviceQueueCreateInfo queue_info = { 0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = *queue_family_index;
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

	VKCall(vkCreateDevice(*gpu, &device_info, 0, device));

	return VK_SUCCESS;
}

VkResult get_surface_format(VkInstance instance, VkPhysicalDevice gpu, VkFormat format, VkSurfaceFormatKHR* surface_format) {
	VkSurfaceKHR surface;
	uint32_t window = window_create(0, 0, 1, 1, " ", 0);
	VKCall(create_vulkan_surface(instance, window, &surface));

	uint32_t format_count = 0;
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, 0));
	VkSurfaceFormatKHR* surface_formats = alloca(format_count * sizeof(VkSurfaceFormatKHR));
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surface_formats));

	for (uint32_t i = 0; i < format_count; i++) {

		if (surface_formats[i].format == format) {
			*surface_format = surface_formats[i];

			VKCall(destroy_vulkan_surface(instance, surface));
			window_destroy(window);

			return VK_SUCCESS;
		}
	}


	VKCall(destroy_vulkan_surface(instance, surface));
	window_destroy(window);

	return 187187187;
}

VkResult new_VkShaderModule(VkDevice device, uint8_t* file_path, VkShaderModule* shader_module) {

	uint32_t shader_size;
	void* shader_data = load_file(file_path, &shader_size);

	VkShaderModuleCreateInfo shader_info = { 0 };
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_info.pCode = shader_data;
	shader_info.codeSize = shader_size;
	VKCall(vkCreateShaderModule(device, &shader_info, 0, shader_module));

	free(shader_data);

	return VK_SUCCESS;
}

VkPipelineShaderStageCreateInfo shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits flag_bits) {
	
	VkPipelineShaderStageCreateInfo shader_stage = { 0 };
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.pName = "main";
	shader_stage.stage = flag_bits;
	shader_stage.module = shader_module;

	return shader_stage;
}

VkResult command_pool_create(VkDevice device, uint32_t queue_index, VkCommandPool* command_pool) {
	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return vkCreateCommandPool(device, &pool_info, 0, command_pool);
}

VkResult command_buffer_allocate(VkDevice device, VkCommandPool command_pool, VkCommandBuffer* cmd) {

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	return vkAllocateCommandBuffers(device, &alloc_info, cmd);
}

VkResult semaphore_create(VkDevice device, VkSemaphore* semaphore) {

	VkSemaphoreCreateInfo sema_info = { 0 };
	sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	return vkCreateSemaphore(device, &sema_info, 0, semaphore);
}

VkResult fence_create(VkDevice device , VkFence* fence) {

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	return vkCreateFence(device, &fence_info, 0, fence);
}