#pragma once

#ifndef PIXELCHAR_RENDERER_BACKEND_VULKAN_H
#define PIXELCHAR_RENDERER_BACKEND_VULKAN_H

#include <vulkan/vulkan.h>
#include "../../pixelchar_renderer_max_pixelfonts.h"

#define PIXELCHAR_VULKAN_STAGING_SIZE 262144

#define VK_CALL_FUNCTION(call, debug, instead) do {if (call != VK_SUCCESS) {debug; instead;}} while(0)

struct pixelchar_renderer_backend_vulkan
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
	void* vertex_index_buffer_host_handle;
	void* staging_buffer_host_handle;

	VkQueue queue;
	uint32_t queue_index;
	VkFence fence;
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd;

	bool update_fonts[PIXELCHAR_RENDERER_MAX_FONTS];
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

void pixelchar_renderer_backend_vulkan_deinit(struct pixelchar_renderer* pcr);

void pixelchar_renderer_backend_vulkan_render(struct pixelchar_renderer* pcr, VkCommandBuffer cmd, uint32_t width, uint32_t height, float shadow_devisor_r, float shadow_devisor_g, float shadow_devisor_b, float shadow_devisor_a);

#endif
