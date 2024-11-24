#include "general/platformlib/platform/platform.h"

#include <stdio.h>
#include <malloc.h>
#include <STB_IMAGE/stb_image.h>

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

VkResult get_first_suitable_VkSurfaceFormatKHR(VkPhysicalDevice gpu, VkSurfaceKHR surface, VkSurfaceFormatKHR* surface_format) {
	uint32_t format_count = 0;
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, 0));
	VkSurfaceFormatKHR* surface_formats = alloca(format_count * sizeof(VkSurfaceFormatKHR));
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surface_formats));

	for (int i = 0; i < format_count; i++) {

		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
			*surface_format = surface_formats[i];
			return VK_SUCCESS;
		}
	}

	return 187187187;
}

VkResult new_SwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkSurfaceFormatKHR surface_format, VkSwapchainKHR* swapchain) {

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));

	uint32_t img_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.minImageCount == surface_capabilities.maxImageCount) img_count = surface_capabilities.minImageCount;

	VkSwapchainCreateInfoKHR sc_info = { 0 };
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_info.surface = surface;
	sc_info.imageFormat = surface_format.format;
	sc_info.preTransform = surface_capabilities.currentTransform;
	sc_info.imageExtent = surface_capabilities.currentExtent;
	sc_info.minImageCount = img_count;
	sc_info.imageArrayLayers = 1;

	VKCall(vkCreateSwapchainKHR(device, &sc_info, 0, swapchain));

	return VK_SUCCESS;
}

VkResult new_VkBuffer(VkDevice device, VkPhysicalDevice gpu, uint32_t size, VkBufferUsageFlags buffer_usage_flags, VkMemoryPropertyFlagBits memory_flags, struct rendering_buffer* buffer) {

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = buffer_usage_flags;
	buffer_info.size = size;

	VKCall(vkCreateBuffer(device, &buffer_info, 0, &buffer->buffer));
	
	VkMemoryRequirements memory_requirements;
	
	vkGetBufferMemoryRequirements(device, buffer->buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & memory_flags) == memory_flags) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = size;

	VKCall(vkAllocateMemory(device, &alloc_info, 0, &buffer->memory));
	return VK_SUCCESS;
}

VkResult new_VkImage(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, uint8_t* file_path, struct rendering_buffer* buffer, struct rendering_image* image) {

	uint32_t width, height, channels;
	uint32_t* img = stbi_load(file_path, &width, &height, &channels, 4);
	if (!img) return 187187187;

	memcpy(buffer->data, img, width * height * 4);

	VkImageCreateInfo image_info = { 0 };
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	image_info.extent = (VkExtent3D) { width, height, 1 };
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VKCall(vkCreateImage(device, &image_info, 0, &image->image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, image->image, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

	VkMemoryAllocateInfo mem_alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = width * width * 4;

	VKCall(vkAllocateMemory(device, &mem_alloc_info, 0, &image->memory));
	VKCall(vkBindImageMemory(device, image->image, image->memory, 0));

	VkCommandBuffer cmd;

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VKCall(vkAllocateCommandBuffers(device, &alloc_info, &cmd));

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(cmd, &begin_info));

	VkImageSubresourceRange range = { 0 };
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.layerCount = 1;
	range.levelCount = 1;

	VkImageMemoryBarrier img_mem_barrier = { 0 };
	img_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	img_mem_barrier.image = image->image;
	img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	img_mem_barrier.subresourceRange = range;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		0,
		0,
		0,
		1,
		&img_mem_barrier
	);

	VkBufferImageCopy copy_region = { 0 };
	copy_region.imageExtent = (VkExtent3D){ width, height, 1 };
	copy_region.imageSubresource.layerCount = 1;
	copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	vkCmdCopyBufferToImage(cmd, buffer->buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

	img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		0,
		0,
		0,
		1,
		&img_mem_barrier
	);

	VKCall(vkEndCommandBuffer(cmd));

	VkFence upload_fence;

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VKCall(vkCreateFence(device, &fence_info, 0, &upload_fence));

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd;

	VKCall(vkQueueSubmit(graphics_queue, 1, &submit_info, upload_fence));

	VKCall(vkWaitForFences(device, 1, &upload_fence, 1, UINT64_MAX));

	VkImageViewCreateInfo view_info = { 0 };
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image->image;
	view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.levelCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VKCall(vkCreateImageView(device, &view_info, 0, &image->view));

	return VK_SUCCESS;
}