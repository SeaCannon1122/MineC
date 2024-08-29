#pragma once

#include "src/general/argb_image.h"

enum alignment {
	ALIGNMENT_LEFT = 0,
	ALIGNMENT_RIGHT = 1,
	ALIGNMENT_TOP = 2,
	ALIGNMENT_BOTTOM = 3,
	ALIGNMENT_MIDDLE = 4,
};

enum menu_item_type {
	MENU_ITEM_LABEL,
	MENU_ITEM_IMAGE,
	MENU_ITEM_BUTTON,
	MENU_ITEM_SLIDER,
	MENU_ITEM_TEXT_FIELD,
};

struct menu_label {
	int z;
	int x;
	int y;
	char alignment_x;
	char alignment_y;
	char text[256];
	unsigned int color;
};

struct menu_image {
	int z;
	int x;
	int y;
	char alignment_x;
	char alignment_y;
	struct arb_image* const image;
};

struct menu_button {
	int z;
	void (*event_function)();
	int x_min;
	int y_min;
	int x_max;
	int y_max;
	char alignment_x;
	char alignment_y;
	unsigned int color;
};

struct menu_slider {
	int z;
	void (*event_function)(float);
	int x_min;
	int y_min;
	int x_max;
	int y_max;
	char alignment_x;
	char alignment_y;
	unsigned int color;
};

struct menu_text_field {
	int z;
	void (*event_function)(char*);
	int x_min;
	int y_min;
	int x_max;
	int y_max;
	char alignment_x;
	char alignment_y;
	char text[256];
	unsigned int color;
};

struct menu_item {
	union {
		struct menu_label label;
		struct menu_button button;
		struct menu_slider slider;
		struct menu_text_field text_field;
	} items;

	char menu_item_type;
};

struct menu_scene {
	struct menu_item menu_items[256];
	
	int menu_items_count;

	struct argb_image* const background;
	float background_scalar;
};

void write_char(char c, unsigned int* screen, int width, int height, int x, int y);

void bind_menu_label(struct menu_scene* scene, struct menu_label* label);

void bind_menu_button(struct menu_scene* scene, struct menu_button* buttom);

void bind_menu_slider(struct menu_scene* scene, struct menu_slider* slider);

void bind_menu_text_field(struct menu_scene* scene, struct menu_text_field* text_field);

void render_menu_scene(struct menu_scene* scene, unsigned int* screen, int width, int height);

