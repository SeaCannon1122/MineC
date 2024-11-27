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
	VkImageView view;
};

struct rendering_buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
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

VkResult new_VkDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* gpu, uint32_t* queue_family_index, float queue_priority, VkDevice* device);

VkResult new_VkSwapchainKHR(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t* images_count, VkSwapchainKHR* swapchain, VkSurfaceFormatKHR* surface_format, VkImage* swapchain_images, VkImageView* swapchain_image_views);

VkResult new_VkShaderModule(VkDevice device, uint8_t* file_path, VkShaderModule* shader_module);

VkPipelineShaderStageCreateInfo shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits flag_bits);

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue graphics_queue, VkCommandPool command_pool, struct rendering_memory_manager* rmm);
VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm);

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer);
VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size);
VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer);

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint8_t* file_path, struct rendering_image* image);
VkResult VkImage_destroy(struct rendering_memory_manager* rmm, struct rendering_image* image);