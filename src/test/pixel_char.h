#pragma once 

#include "vulkan/vulkan.h"

#if defined(_WIN32)

#ifndef RESTRICT
#define RESTRICT __restrict
#endif

#elif defined(__linux__)

#ifndef RESTRICT
#define RESTRICT restrict
#endif

#endif

#ifndef PIXEL_FONT_RESOLUTION
#define PIXEL_FONT_RESOLUTION 16
#endif

#ifndef PIXEL_FONT_SHADOW_DIVISOR
#define PIXEL_FONT_SHADOW_DIVISOR 3
#endif

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
		char layout[PIXEL_FONT_RESOLUTION * PIXEL_FONT_RESOLUTION / 8];
	} char_font_entries[0x20000];
};

struct pixel_char {
	float color[4];
	float background_color[4];
	uint64_t value;
	uint64_t masks;
};


#define pixel_char_convert_string(name, str, color, background_color, masks) struct pixel_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

#define pixel_char_convert_string_in(name, str, color, background_color, masks) {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

struct pixel_font* load_pixel_font(char* src);

uint32_t pixel_char_get_hover_index(const struct pixel_char* RESTRICT string, uint32_t text_size, int32_t line_spacing, int32_t x, int32_t y, int32_t alignment_x, int32_t alignment_y, int32_t max_width, uint32_t max_lines, const const void** RESTRICT font_map, int x_hover, int y_hover);

//int pixel_char_fitting(const struct pixel_char* RESTRICT string, uint32_t text_size, const const void** RESTRICT font_map, int32_t max_width);

void pixel_char_print_string(struct pixel_char_renderer* RESTRICT renderer, const struct pixel_char* RESTRICT string, uint32_t text_size, int32_t line_spacing, int32_t x, int32_t y, int32_t alignment_x, int32_t alignment_y, int32_t max_width, uint32_t max_lines, uint32_t width, uint32_t height, const const void** RESTRICT font_map);
