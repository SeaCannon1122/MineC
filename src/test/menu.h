#pragma once

#include "general/argb_image.h"
#include "general/platformlib/platform.h"

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

enum alignment {
	MENU_ALIGNMENT_LEFT,
	MENU_ALIGNMENT_RIGHT,
	MENU_ALIGNMENT_TOP,
	MENU_ALIGNMENT_BOTTOM,
	MENU_ALIGNMENT_MIDDLE,
};

enum menu_item_type {
	MENU_ITEM_LABEL,
	MENU_ITEM_IMAGE,
	MENU_ITEM_SLIDER,
	MENU_ITEM_TEXT_FIELD,
};

struct menu_label {
	int x;
	int y;
	char alignment_x;
	char alignment_y;
	char text_alignment_x;
	char text_alignment_y;
	struct gui_char* text;
};

struct menu_image {
	int x;
	int y;
	char alignment_x;
	char alignment_y;
	char image_alignment_x;
	char image_alignment_y;
	int image;
	int image_scalar;
};

struct menu_slider {
	float state;
	int x_min;
	int y_min;
	int x_max;
	int y_max;
	char alignment_x;
	char alignment_y;
	int texture_background;
	int texture_slider;
	int slider_thickness;
};

struct menu_text_field {
	char* buffer;
	int x_min;
	int x_max;
	int y;
	char alignment_x;
	char alignment_y;
	char text_alignment;
	char* selected;
	int font;
	char* field_visible;
};


struct menu_item {
	union {
		struct menu_label label;
		struct menu_image image;
		struct menu_slider slider;
		struct menu_text_field text_field;
	} items;
	int z;
	char menu_item_type;
};

#define gui_string_convert(name, str, color, underline_cursive_font) struct gui_char name[sizeof(str)]; {for(int _gsc_i = 0; _gsc_i < sizeof(str); _gsc_i++) name[_gsc_i] = (struct gui_char) {color, str[_gsc_i], underline_cursive_font, NULL};}

void gui_print_char(const struct gui_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map);
void gui_print_string(const struct gui_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map);

void menu_frame(struct menu_item** menu_items, int menu_items_count, unsigned int* screen, int width, int height, int scale, void** resource_map);