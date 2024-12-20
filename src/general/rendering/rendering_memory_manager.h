#ifndef RENDERING_MEMORY_MANAGER_H
#define RENDERING_MEMORY_MANAGER_H

#include "vulkan_helpers.h"

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
	void* memory_host_handle;
	VkMemoryPropertyFlags property_flags;
	uint32_t size;
};

struct rendering_memory_manager {
	VkDevice device;
	VkPhysicalDevice gpu;
	VkQueue queue;
	uint32_t queue_index;
	VkCommandPool command_pool;

	VkCommandBuffer cmd;
	VkFence cmd_fence;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	void* staging_buffer_host_handle;
};

VkResult rendering_memory_manager_new(VkDevice device, VkPhysicalDevice gpu, VkQueue queue, uint32_t queue_index, size_t staging_buffer_size, struct rendering_memory_manager* rmm);
VkResult rendering_memory_manager_destroy(struct rendering_memory_manager* rmm);

VkResult VkBuffer_new(struct rendering_memory_manager* rmm, uint32_t size, VkMemoryPropertyFlags property_flags, VkBufferUsageFlags usage_flags, struct rendering_buffer* buffer);
VkResult VkBuffer_fill(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer, void* data, uint32_t size, uint32_t offset);
VkResult VkBuffer_destroy(struct rendering_memory_manager* rmm, struct rendering_buffer* buffer);

VkResult VkImage_new(struct rendering_memory_manager* rmm, uint32_t width, uint32_t height, VkFormat image_format, uint32_t pixel_size, VkImageUsageFlags usage, struct rendering_image* image);
VkResult VkImage_fill(struct rendering_memory_manager* rmm, void* image_data, VkImageUsageFlags usage, struct rendering_image* image);
VkResult VkImage_destroy(struct rendering_memory_manager* rmm, struct rendering_image* image);

#endif // RENDERING_MEMORY_MANAGER_H
