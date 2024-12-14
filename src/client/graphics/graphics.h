#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "general/rendering/vulkan_helpers.h"
#include "general/rendering/rendering_memory_manager.h"
#include "general/rendering/rendering_window.h"

#include "graphics_device.h"

struct graphics_state {
	
	//static
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
};

#endif // !GRAPHICS_H

struct game_client;

uint32_t graphics_create(struct game_client* game);
uint32_t graphics_destroy(struct game_client* game);