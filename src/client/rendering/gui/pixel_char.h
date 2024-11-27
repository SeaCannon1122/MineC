#pragma once 

#include "client/rendering/rendering_context.h"

#include <stdint.h>

#define MAX_PIXEL_FONTS 4

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x000000ff


struct pixel_font {
	struct {
		uint64_t width;
		int layout[8];
	} char_font_entries[0x20000];
};

struct pixel_char_renderer {
	VkDevice device;

	VkDescriptorSetLayout set_layout;
	VkDescriptorSet descriptor_set;

	VkPipelineLayout pipe_layout;
	VkPipeline pipeline;
	
	VkDescriptorPool descriptor_pool;

	struct rendering_buffer pixel_char_buffer;
	struct rendering_buffer pixel_font_buffer[MAX_PIXEL_FONTS];
	uint32_t font_count;

	uint32_t chars_to_draw;
};

struct pixel_char {
	float color[4];
	float background_color[4];
	uint64_t value;
	uint64_t masks;
};

struct pixel_render_char {
	uint64_t size;
	float start_position[2];
	struct pixel_char pixel_char_data;
};

struct pixel_font* load_pixel_font(char* src);

uint32_t pixel_char_renderer_new(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, VkDevice device, VkRenderPass render_pass);

uint32_t pixel_char_renderer_add_font(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, struct pixel_font* font_data);

uint32_t pixel_char_renderer_fill_chars(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, struct pixel_render_char* chars, uint32_t chars_count);

uint32_t pixel_char_renderer_render(struct pixel_char_renderer* pcr, VkCommandBuffer cmd, VkExtent2D screen_size);