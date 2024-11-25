#include "general/platformlib/platform/platform.h"

#include <stdio.h>
#include <malloc.h>
#include <STB_IMAGE/stb_image.h>

#include "rendering_context.h"

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

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
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

VkResult new_VkDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device) {

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

					*queue_family_index = j;
					*gpu = gpus[i];

					goto gpu_found;
				}

			}

		}

	}

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

VkResult new_VkSwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t* images_count, VkSwapchainKHR* swapchain, VkSurfaceFormatKHR* surface_format, VkImage* swapchain_images, VkImageView* swapchain_image_views) {

	uint32_t format_count = 0;
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, 0));
	VkSurfaceFormatKHR* surface_formats = alloca(format_count * sizeof(VkSurfaceFormatKHR));
	VKCall(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surface_formats));


	uint32_t found = 0;

	for (uint32_t i = 0; i < format_count; i++) {

		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
			*surface_format = surface_formats[i];
			found = 1;
			break;
		}
	}

	if(found == 0) return 187187187;

	VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
	VKCall(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities));
	
	VkSwapchainCreateInfoKHR sc_info = { 0 };
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_info.surface = surface;
	sc_info.imageFormat = surface_format->format;
	sc_info.preTransform = surface_capabilities.currentTransform;
	sc_info.imageExtent = surface_capabilities.currentExtent;
	sc_info.minImageCount = *images_count;
	sc_info.imageArrayLayers = 1;

	VKCall(vkCreateSwapchainKHR(device, &sc_info, 0, swapchain));

	VKCall(vkGetSwapchainImagesKHR(device, *swapchain, images_count, swapchain_images));

	VkImageViewCreateInfo image_view_info = { 0 };
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.format = surface_format->format;
	image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.levelCount = 1;

	for (uint32_t i = 0; i < *images_count; i++) {

		image_view_info.image = swapchain_images[i];
		VKCall(vkCreateImageView(device, &image_view_info, 0, &swapchain_image_views[i]));
	}

	return VK_SUCCESS;
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

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, struct rendering_memory_manager* rmm) {

	rmm->command_pool = command_pool;
	rmm->device = device;
	rmm->gpu = gpu;
	rmm->graphics_queue = graphics_queue;

	//staging buffer

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.size = 10000000;

	VKCall(vkCreateBuffer(device, &buffer_info, 0, &rmm->staging_buffer));

	VkMemoryRequirements memory_requirements;

	vkGetBufferMemoryRequirements(device, rmm->staging_buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = 10000000;

	VKCall(vkAllocateMemory(device, &alloc_info, 0, &rmm->staging_buffer_memory));

	VKCall(vkBindBufferMemory(device, rmm->staging_buffer, rmm->staging_buffer_memory, 0));
	VKCall(vkMapMemory(device, rmm->staging_buffer_memory, 0, 10000000, 0, &rmm->staging_buffer_host_handle));

	//command buffer

	VkCommandBufferAllocateInfo cmd_alloc_info = { 0 };
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.commandPool = command_pool;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VKCall(vkAllocateCommandBuffers(device, &cmd_alloc_info, &rmm->cmd));

	VKCall(vkResetCommandBuffer(rmm->cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

	return VK_SUCCESS;
}

VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm) {

	vkUnmapMemory(rmm->device, rmm->staging_buffer_memory);
	
	vkFreeMemory(rmm->device, rmm->staging_buffer_memory, 0);
	vkDestroyBuffer(rmm->device, rmm->staging_buffer, 0);
	vkFreeCommandBuffers(rmm->device, rmm->command_pool, 1, &rmm->cmd);

	return VK_SUCCESS;
}

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer) {

	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = usage_flags |VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_info.size = size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCall(vkCreateBuffer(rmm->device, &buffer_info, 0, &buffer->buffer));
	
	VkMemoryRequirements memory_requirements;
	
	vkGetBufferMemoryRequirements(rmm->device, buffer->buffer, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(rmm->gpu, &memory_properties);

	VkMemoryAllocateInfo alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = (size > memory_requirements.size ? size : memory_requirements.size);

	VKCall(vkAllocateMemory(rmm->device, &alloc_info, 0, &buffer->memory));
	VKCall(vkBindBufferMemory(rmm->device, buffer->buffer, buffer->memory, 0));

	return VK_SUCCESS;
}

VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size) {

	memcpy(rmm->staging_buffer_host_handle, data, size);

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(rmm->cmd, &begin_info));

	VkBufferCopy copy_region = { 0 };
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;

	vkCmdCopyBuffer(rmm->cmd, rmm->staging_buffer, buffer->buffer, 1, &copy_region);

	VKCall(vkEndCommandBuffer(rmm->cmd));

	VkFence upload_fence;

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VKCall(vkCreateFence(rmm->device, &fence_info, 0, &upload_fence));

	VKCall(vkResetFences(rmm->device, 1, &upload_fence));

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &rmm->cmd;

	VKCall(vkQueueSubmit(rmm->graphics_queue, 1, &submit_info, upload_fence));

	VKCall(vkWaitForFences(rmm->device, 1, &upload_fence, 1, UINT64_MAX));

	vkDestroyFence(rmm->device, upload_fence, 0);

	return VK_SUCCESS;
}

VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer) {
	vkDestroyBuffer(rmm->device, buffer->buffer, 0);
	vkFreeMemory(rmm->device, buffer->memory, 0);

	return VK_SUCCESS;
}

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint8_t* file_path, struct rendering_image* image) {

	uint32_t width, height, channels;
	uint32_t* img = stbi_load(file_path, &width, &height, &channels, 4);
	if (!img) return 187187187;

	memcpy(rmm->staging_buffer_host_handle, img, width * height * 4);

	free(img);

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

	VKCall(vkCreateImage(rmm->device, &image_info, 0, &image->image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(rmm->device, image->image, &memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(rmm->gpu, &memory_properties);

	VkMemoryAllocateInfo mem_alloc_info = { 0 };
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((memory_requirements.memoryTypeBits & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc_info.memoryTypeIndex = i;
			break;
		}
	}

	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = width * width * 4;

	VKCall(vkAllocateMemory(rmm->device, &mem_alloc_info, 0, &image->memory));
	VKCall(vkBindImageMemory(rmm->device, image->image, image->memory, 0));


	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKCall(vkBeginCommandBuffer(rmm->cmd, &begin_info));

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
		rmm->cmd,
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

	vkCmdCopyBufferToImage(rmm->cmd, rmm->staging_buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

	img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	img_mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		rmm->cmd,
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

	VKCall(vkEndCommandBuffer(rmm->cmd));

	VkFence upload_fence;

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VKCall(vkCreateFence(rmm->device, &fence_info, 0, &upload_fence));

	VKCall(vkResetFences(rmm->device, 1, &upload_fence));

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &rmm->cmd;

	VKCall(vkQueueSubmit(rmm->graphics_queue, 1, &submit_info, upload_fence));

	VKCall(vkWaitForFences(rmm->device, 1, &upload_fence, 1, UINT64_MAX));

	vkDestroyFence(rmm->device, upload_fence, 0);

	VkImageViewCreateInfo view_info = { 0 };
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image->image;
	view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.levelCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VKCall(vkCreateImageView(rmm->device, &view_info, 0, &image->view));

	return VK_SUCCESS;
}

VkResult VkImage_destroy(struct rendering_memory_manager* rmm, struct rendering_image* image) {
	vkDestroyImage(rmm->device, image->image, 0);
	vkFreeMemory(rmm->device, image->memory, 0);
	vkDestroyImageView(rmm->device, image->view, 0);
}