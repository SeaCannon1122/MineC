#pragma once 

#if defined(_WIN32)

#ifndef _PIXEL_CHAR_RESTRICT
#define _PIXEL_CHAR_RESTRICT __restrict
#endif

#elif defined(__linux__)

#ifndef _PIXEL_CHAR_RESTRICT
#define _PIXEL_CHAR_RESTRICT restrict
#endif

#endif

#ifndef PIXEL_FONT_RESOULUTION
#define PIXEL_FONT_RESOULUTION 16
#endif

#ifndef PIXEL_FONT_SHADOW_DIVISOR
#define PIXEL_FONT_SHADOW_DIVISOR 4
#endif

#define PIXEL_CHAR_UNDERLINE_MASK  0x80000000
#define PIXEL_CHAR_CURSIVE_MASK    0x40000000
#define PIXEL_CHAR_SHADOW_MASK     0x20000000
#define PIXEL_CHAR_BACKGROUND_MASK 0x10000000
#define PIXEL_CHAR_FONT_MASK       0x0fffffff

#define PIXEL_CHAR_ALIGNMENT_LEFT   0
#define PIXEL_CHAR_ALIGNMENT_RIGHT  1
#define PIXEL_CHAR_ALIGNMENT_TOP    2
#define PIXEL_CHAR_ALIGNMENT_BOTTOM 3
#define PIXEL_CAHR_ALIGNMENT_MIDDLE 5


struct pixel_font_entry {
	long long width;
	char layout[PIXEL_FONT_RESOULUTION * PIXEL_FONT_RESOULUTION / 8];
};

struct pixel_font {
	struct pixel_font_entry char_font_entries[2097152];
};

struct pixel_char {
	unsigned int color;
	unsigned int background_color;
	unsigned int value;
	unsigned int masks;
};

#define pixel_char_convert_string(name, str, color, background_color, masks) struct pixel_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, background_color, str[_gsc_i], masks};}

void pixel_char_print_string(const struct pixel_char* _PIXEL_CHAR_RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, unsigned int* _PIXEL_CHAR_RESTRICT screen, int width, int height, const const void** _PIXEL_CHAR_RESTRICT font_map);

int pixel_char_get_hover_index(const struct pixel_char* _PIXEL_CHAR_RESTRICT string, int text_size, int line_spacing, int x, int y, int alignment_x, int alignment_y, const const void** _PIXEL_CHAR_RESTRICT font_map, int x_hover, int y_hover);

int pixel_char_fitting(const struct pixel_char* _PIXEL_CHAR_RESTRICT string, int text_size, const const void** _PIXEL_CHAR_RESTRICT font_map, int max_width);