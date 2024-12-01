#pragma once

#include <vulkan/vulkan.h>


struct rendering_window {
	uint32_t window;

	uint32_t width;
	uint32_t height;

	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkSurfaceFormatKHR surface_format;

	VkSwapchainKHR swapchain;
	uint32_t sc_image_count;
	VkImage sc_images[5];
	VkImageView sc_image_views[5];
	VkFramebuffer framebuffers[5];

	VkRenderPass render_pass;
};

int32_t rendering_window_new(struct rendering_window* window, VkInstance instance, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name);

int32_t rendering_window_swapchain_create(struct rendering_window* window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceFormatKHR surface_format, VkRenderPass render_pass);
int32_t rendering_window_swapchain_destroy(struct rendering_window* window);

int32_t rendering_window_resize(struct rendering_window* window, uint32_t new_width, uint32_t new_height);