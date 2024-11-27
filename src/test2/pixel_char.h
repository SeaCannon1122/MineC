#pragma once 

#include "client/rendering/rendering_context.h"

#include <stdint.h>

#if defined(_WIN32)

#ifndef RESTRICT
#define RESTRICT __restrict
#endif

#elif defined(__linux__)

#ifndef RESTRICT
#define RESTRICT restrict
#endif

#endif

#define MAX_PIXEL_FONTS 4

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x000000ff

#ifndef ALIGNMENTS
#define ALIGNMENTS

#define ALIGNMENT_LEFT   0
#define ALIGNMENT_RIGHT  1
#define ALIGNMENT_TOP    2
#define ALIGNMENT_BOTTOM 3
#define ALIGNMENT_MIDDLE 5

#endif // !ALIGNMENTS

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


#define pixel_char_convert_string(name, str, color, background_color, masks) struct pixel_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

#define pixel_char_convert_string_in(name, str, color, background_color, masks) {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

struct pixel_font* load_pixel_font(char* src);

uint32_t pixel_char_renderer_new(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, VkDevice device, VkRenderPass render_pass);

uint32_t pixel_char_renderer_add_font(struct pixel_char_renderer* pcr, struct rendering_memory_manager* rmm, struct pixel_font* font_data);