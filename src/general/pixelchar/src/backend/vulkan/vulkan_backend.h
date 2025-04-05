#pragma once

#ifndef PIXELCHAR_VULKAN_BACKEND_H
#define PIXELCHAR_VULKAN_BACKEND_H

#include <vulkan/vulkan.h>

struct pixelchar_vulkan_backend
{
	VkDevice device;
	VkPhysicalDevice gpu;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;

	VkPipelineLayout pipe_layout;
	VkPipeline pipeline;

	VkDescriptorPool descriptor_pool;

	VkBuffer pixel_char_buffer;
	VkDeviceMemory pixel_char_buffer_memory;
	void* pixel_char_buffer_host_handle;

	uint32_t cmd_set;
	VkCommandBuffer cmd;
};
struct pixelchar_renderer;

uint32_t pixelchar_renderer_backend_vulkan_init(
	struct pixelchar_renderer* pcr, 
	VkDevice device,
	VkPhysicalDevice gpu,
	VkRenderPass render_pass,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

void pixelchar_renderer_backend_vulkan_set_command_buffer(struct pixelchar_renderer* pcr, VkCommandBuffer cmd);

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

uint32_t _pixelchar_renderer_backend_vulkan_add_font(struct pixelchar_renderer* pcr, struct pixelfont* font, uint32_t index);

void _pixelchar_renderer_backend_vulkan_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height);

void _pixelchar_renderer_backend_vulkan_deinit(struct pixelchar_renderer* pcr);

#endif


#endif
