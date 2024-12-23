#pragma once 

#include "general/rendering/vulkan_helpers.h"
#include <stdint.h>

#define MAX_PIXEL_FONTS 4

#define PIXEL_CHAR_UNDERLINE_MASK  0x8000
#define PIXEL_CHAR_CURSIVE_MASK    0x4000
#define PIXEL_CHAR_SHADOW_MASK     0x2000
#define PIXEL_CHAR_BACKGROUND_MASK 0x1000
#define PIXEL_CHAR_FONT_MASK       0x00ff


struct pixel_font {
	struct {
		uint64_t width;
		int layout[8];
	} char_font_entries[0x20000];
};

struct pixel_char_renderer {
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

	uint32_t chars_to_draw;
};

struct pixel_render_char {
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t value;
	uint16_t position[2];
	uint16_t masks;
	int16_t size;
};

struct pixel_font* load_pixel_font(char* src);

uint32_t pixel_char_renderer_new(struct pixel_char_renderer* pcr, VkDevice device, VkPhysicalDevice gpu, VkRenderPass render_pass);
uint32_t pixel_char_renderer_destroy(struct pixel_char_renderer* pcr);

uint32_t pixel_char_renderer_add_font(struct pixel_char_renderer* pcr, VkBuffer buffer, uint32_t font_index);

uint32_t pixel_char_renderer_fill_chars(struct pixel_char_renderer* pcr, struct pixel_render_char* chars, uint32_t chars_count);

uint32_t pixel_char_renderer_render(struct pixel_char_renderer* pcr, VkCommandBuffer cmd, VkExtent2D screen_size);