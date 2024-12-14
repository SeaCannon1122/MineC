#pragma once

#include <vulkan/vulkan.h>

struct rendering_window_state {
	uint32_t window;

	uint32_t width;
	uint32_t height;

	VkInstance instance;

	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkSurfaceFormatKHR surface_format;

	VkSwapchainKHR swapchain;
	uint32_t sc_image_count;
	VkImage sc_images[5];
	VkImageView sc_image_views[5];
	VkFramebuffer framebuffers[5];
	uint32_t sc_image_index;

	VkRenderPass render_pass;
};

int32_t rendering_window_create_rendering_pass(VkDevice device, VkSurfaceFormatKHR surface_format, VkRenderPass* render_pass);
int32_t rendering_window_renderpass_begin_info(uint32_t window, VkRenderPassBeginInfo* renderpass_begin_info, VkRenderPass render_pass, VkSemaphore aquire_semaphore);
int32_t rendering_window_present_image(uint32_t window, VkQueue queue, VkSemaphore* wait_semaphores, uint32_t semaphore_count);

int32_t rendering_window_new(uint32_t window, VkInstance instance);
int32_t rendering_window_destroy(uint32_t window);

int32_t rendering_window_swapchain_create(uint32_t window, VkPhysicalDevice gpu, VkDevice device, VkSurfaceFormatKHR surface_format, VkRenderPass render_pass);
int32_t rendering_window_swapchain_destroy(uint32_t window);

int32_t rendering_window_resize(uint32_t window);

int32_t rendering_window_get_data(int32_t window, struct rendering_window_state* state);