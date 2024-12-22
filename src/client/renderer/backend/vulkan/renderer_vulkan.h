#ifndef RENDERER_VULKAN_H
#define RENDERER_VULKAN_H

#include <stdint.h>
#include "general/platformlib/platform/platform.h"
#include "general/rendering/rendering_memory_manager.h"
#include "client/resources/resources.h"

struct renderer_backend {

	VkPhysicalDevice gpus[16];
	uint32_t gpu_queue_indices[16];
	VkSurfaceFormatKHR gpu_surface_formats[16];


	int32_t dummy_window;
	VkSurfaceFormatKHR surface_format;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	//Device
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue queue;
	uint32_t queue_index;

	struct rendering_memory_manager rmm;

	VkCommandPool command_pool;
	VkCommandBuffer cmd;

	VkSemaphore submit_semaphore;
	VkSemaphore aquire_semaphore;
	VkFence img_available_fence;

	VkRenderPass window_render_pass;

	struct {
		VkImage image;
		VkImageView image_view;
	} image_atlas[RESOURCES_IMAGES_COUNT];
	VkDeviceMemory image_memory;

};

#endif // !RENDERER_VULKAN_H
