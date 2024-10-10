#pragma once 

#if defined(_WIN32)

#ifndef RESTRICT
#define RESTRICT __restrict
#endif

#elif defined(__linux__)

#ifndef RESTRICT
#define RESTRICT restrict
#endif

#endif

#ifndef IF_BIT
#define IF_BIT(ptr, pos) (((char*)ptr)[pos / 8] & (1 << (pos % 8)) )
#endif

#ifndef SET_BIT_1
#define SET_BIT_1(ptr, pos) (((char*)ptr)[pos / 8] |= (1 << (pos % 8) ))
#endif

#ifndef SET_BIT_0
#define SET_BIT_0(ptr, pos) (((char*)ptr)[pos / 8] &= (0xFF ^ (1 << (pos % 8) )))
#endif

#define PIXEL_FONT_RESOULUTION 16

#define PIXEL_CHAR_UNDERLINE_MASK 0x80000000
#define PIXEL_CHAR_CURSIVE_MASK   0x40000000
#define PIXEL_CHAR_FONT_MASK      0x3fffffff

struct pixel_font_entry {
	long long width;
	char layout[PIXEL_FONT_RESOULUTION * PIXEL_FONT_RESOULUTION / 8];
};

struct pixel_font {
	struct pixel_font_entry char_font_entries[2097152];
};

struct pixel_char {
	unsigned int color;
	int value;
	int underline_cursive_font;
	char* hover;
};

#define pixel_char_convert_string(name, str, color, underline_cursive_font) struct pixel_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct pixel_char) {color, str[_gsc_i], underline_cursive_font, NULL};}

void pixel_char_print(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map);
void pixel_char_print_string(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map);