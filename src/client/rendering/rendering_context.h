#pragma once

#include "general/platformlib/platform/platform.h"

#define VKCall(call) \
do { \
    VkResult result = (call); \
    if (result != VK_SUCCESS) { \
        printf("Vulkan error in \n    %s \n at %s:%d: %d\n", #call, __FILE__, __LINE__, result); \
		DEBUG_BREAK();\
    } \
} while(0)

struct rendering_image {
	VkImage image;
	VkDeviceMemory memory;
	uint16_t width;
	uint16_t height;
	uint32_t pixel_size;
	VkImageView view;
};

struct rendering_buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint32_t size;
};

struct rendering_memory_manager {
	VkDevice device;
	VkPhysicalDevice gpu;
	VkQueue graphics_queue;
	VkCommandPool command_pool;

	VkCommandBuffer cmd;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	void* staging_buffer_host_handle;
};

VkResult new_VkInstance(uint8_t* app_name, uint8_t* engine_name, VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger);

VkResult new_VkDevice(VkInstance instance, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device);

VkResult get_surface_format(VkInstance instance, VkPhysicalDevice gpu, VkFormat format, VkSurfaceFormatKHR* surface_format);

VkResult new_VkShaderModule(VkDevice device, uint8_t* file_path, VkShaderModule* shader_module);

VkPipelineShaderStageCreateInfo shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits flag_bits);

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, struct rendering_memory_manager* rmm);
VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm);

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer);
VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size);
VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer);

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint32_t width, uint32_t height, VkFormat image_format, uint32_t pixel_size, VkImageUsageFlags usage, struct rendering_image* image);
VkResult VkImage_fill(struct rendering_memory_manager* rmm, void* image_data, VkImageUsageFlags usage, struct rendering_image* image);
VkResult VkImage_destroy(struct rendering_memory_manager* rmm, struct rendering_image* image);