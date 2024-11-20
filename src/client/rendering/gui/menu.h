#pragma once

#include "client/rendering/gui/pixel_char.h"


#ifndef ALIGNMENTS
#define ALIGNMENTS

#define ALIGNMENT_LEFT   0
#define ALIGNMENT_RIGHT  1
#define ALIGNMENT_TOP    2
#define ALIGNMENT_BOTTOM 3
#define ALIGNMENT_MIDDLE 5

#endif // !ALIGNMENTS

enum menu_item_type {
	MENU_ITEM_LABEL,
	MENU_ITEM_IMAGE,
	MENU_ITEM_SLIDER,
	MENU_ITEM_TEXT_FIELD,
};

struct menu_label {
	int menu_item_type;
	int z;
	int x;
	int y;
	int alignment_x;
	int alignment_y;
	int text_alignment_x;
	int text_alignment_y;
	int max_width;
	int max_rows;
	int selectable;
	int hoverable;
	int text_size;
	struct pixel_char text[];
};

struct menu_image {
	int menu_item_type;
	int z;
	int x;
	int y;
	int alignment_x;
	int alignment_y;
	int image_alignment_x;
	int image_alignment_y;
	int hoverable;
	int image;
	int hover_image;
	int image_scalar;
};

struct menu_slider {
	int menu_item_type;
	int z;
	int x;
	int y;
	int alignment_x;
	int alignment_y;
	int image_alignment_x;
	int image_alignment_y;
	int hoverable;
	int slide_image;
	int slider_image;
	int slider_hover_image;
	int image_scalar;
};

struct menu_text_field {
	int menu_item_type;
	int z;
	char* buffer;
	int x_min;
	int x_max;
	int y;
	int alignment_x;
	int alignment_y;
	int text_alignment;
	char* selected;
	int font;
	char* field_visible;
};

struct menu_scene {
	int current_item;

	int select_label;
	int select_begin;
	int select_end;
	int selecting;

	int current_pos;

	int image_pos_x;
	int image_pox_y;
	int image_index;

	int items_count;
	int* items[];
};

void menu_init();

void menu_scene_frame(struct menu_scene* menu, unsigned int* screen, int width, int height, int scale, const void** resource_map, int mouse_x, int mouse_y, int mouse_left_click);

void menu_exit();