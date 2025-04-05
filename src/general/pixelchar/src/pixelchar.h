#pragma once 

#define _PIXELCHAR_INTERNAL_EXCLUDE

#include "pixelchar_renderer.h"
#include "pixelfont.h"

enum pixelchar_mask {
	PIXELCHAR_MASK_UNDERLINE  = 0x8000,
	PIXELCHAR_MASK_CURSIVE    = 0x4000,
	PIXELCHAR_MASK_SHADOW     = 0x2000,
	PIXELCHAR_MASK_BACKGROUND = 0x1000,
	PIXELCHAR_MASK_FONT       = 0x00ff,
};

enum pixelchar_return_status {
	PIXELCHAR_ERROR_VK_VERTEX_SHADER              = 0b10000001,
	PIXELCHAR_ERROR_VK_FRAGMENT_SHADER            = 0b10000010,
	PIXELCHAR_ERROR_VK_PIPELINE                   = 0b10000011,
	PIXELCHAR_ERROR_TOO_MANY_CHARACTERS           = 0b10100000,
	PIXELCHAR_ERROR_FONT_INDEX_OUT_OF_BOUNDS      = 0b11000000,
	PIXELCHAR_ERROR                               = 0b10000000,
};

struct pixelchar_renderer_vk {
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
	uint32_t buffer_length;

	uint32_t chars_to_draw;
};
 
uint32_t pixelchar_set_debug_callback(PIXELCHAR_DEBUG_CALLBACK_FUNCTION callback_function);

uint32_t pixelchar_renderer_vk_new(
	struct pixelchar_renderer_vk* pcr,
	VkDevice device,
	VkPhysicalDevice gpu,
	VkRenderPass render_pass,
	uint32_t buffer_length,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

uint32_t pixelchar_renderer_vk_destroy(struct pixelchar_renderer_vk* pcr);

uint32_t pixelchar_renderer_vk_add_font(struct pixelchar_renderer_vk* pcr, VkBuffer buffer, uint32_t offset, uint32_t font_index);

uint32_t pixelchar_renderer_vk_add_chars(struct pixelchar_renderer_vk* pcr, struct pixelchar* chars, uint32_t chars_count);

uint32_t pixelchar_renderer_vk_cmd_render(struct pixelchar_renderer_vk* pcr, VkCommandBuffer cmd, VkExtent2D screen_size);