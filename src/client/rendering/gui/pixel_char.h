#pragma once 

#define PIXEL_CHAR_DEBUG

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
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

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
		long long width;
		char layout[PIXEL_FONT_RESOLUTION * PIXEL_FONT_RESOLUTION / 8];
	} char_font_entries[0x20000];
};

struct pixel_char {
	unsigned int color;
	unsigned int background_color;
	unsigned int value;
	unsigned int masks;
};

#define pixel_char_convert_string(name, str, color, background_color, masks) struct pixel_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

#define pixel_char_convert_string_in(name, str, color, background_color, masks) {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

struct pixel_font* load_pixel_font(char* src);

void pixel_char_print_string(const struct pixel_char* RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, int max_width, int max_lines, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT font_map);

int pixel_char_get_hover_index(const struct pixel_char* RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, int max_width, int max_lines, const const void** RESTRICT font_map, int x_hover, int y_hover);

int pixel_char_fitting(const struct pixel_char* RESTRICT string, int text_size, const const void** RESTRICT font_map, int max_width);