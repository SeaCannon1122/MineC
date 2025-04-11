#pragma once

#ifndef PIXELCHAR_VULKAN_BACKEND_H
#define PIXELCHAR_VULKAN_BACKEND_H

#include <vulkan/vulkan.h>
#include "pixelchar_renderer_max_pixelfonts.h"

struct pixelchar_vulkan_backend
{
	VkDevice device;
	VkPhysicalDevice gpu;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;

	VkPipelineLayout pipe_layout;
	VkPipeline pipeline;

	VkDescriptorPool descriptor_pool;

	VkBuffer vertex_index_buffer;
	VkBuffer staging_buffer;
	VkDeviceMemory vertex_index_staging_buffer_memory;
	void* vertex_index_staging_buffer_host_handle;

	VkQueue queue;
	uint32_t queue_index;
	VkCommandPool transfer_command_pool;
	VkCommandBuffer transfer_cmd;

	struct
	{
		bool init;

		VkBuffer bitmap_buffer;
		VkDeviceMemory bitmap_buffer_memory;

	} font_info[PIXELCHAR_RENDERER_MAX_FONTS];
};
struct pixelchar_renderer;

uint32_t pixelchar_renderer_backend_vulkan_init(
	struct pixelchar_renderer* pcr,
	VkDevice device,
	VkPhysicalDevice gpu,
	VkQueue queue,
	uint32_t queue_index,
	VkRenderPass render_pass,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

void pixelchar_renderer_backend_vulkan_render(struct pixelchar_renderer* pcr, VkCommandBuffer cmd, uint32_t width, uint32_t height);

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

void _pixelchar_renderer_backend_vulkan_deinit(struct pixelchar_renderer* pcr);

#endif


#endif
