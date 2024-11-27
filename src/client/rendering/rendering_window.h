#pragma once

#include <vulkan/vulkan.h>

struct rendering_core {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
};

struct rendering_window {
	uint32_t window;

	uint32_t width;
	uint32_t height;

	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkSurfaceFormatKHR surface_format;

	VkQueue graphics_queue;
	uint32_t queue_index;

	VkSwapchainKHR swapchain;
	uint32_t sc_image_count;
	VkImage sc_images[5];
	VkImageView sc_image_views[5];
};

int32_t rendering_window_new(struct rendering_window* window, struct rendering_core* core, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name);