#include <stdlib.h>
#include <stdio.h>

#include "src/client/menu.h"

struct char_font {
	char width;
	char font[5 * 7];
};

struct char_font menu_fonts[256];

void init_fonts() {
	char font_exclamation[35] = { 1, 1, 1, 1, 1, 0, 1 };
	menu_fonts[33].width = 1;
	for (int i = 0; i < 35; i++) menu_fonts[33].font[i] = font_exclamation[i];
}

void write_char(char c, unsigned int* screen, int width, int height, int x, int y) {
	
}

void bind_menu_label(struct menu_scene* scene, struct menu_label* label) {

}

void bind_menu_button(struct menu_scene* scene, struct menu_button* buttom) {

}

void bind_menu_slider(struct menu_scene* scene, struct menu_slider* slider) {

}

void bind_menu_text_field(struct menu_scene* scene, struct menu_text_field* text_field) {

}

int compare(const void* a, const void* b) {

	return ((*(struct menu_item**)a)->items.button.z - (*(struct menu_item**)b)->items.button.z);
}

void render_menu_scene(struct menu_scene* scene, unsigned int* screen, int width, int height) {

	union argb_pixel* ptr = screen;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {

			union argb_pixel top;
			top.color_value = scene->background->pixels[ ((int)(i / scene->background_scalar)) % scene->background->width + scene->background->width * (((int)(j / scene->background_scalar)) % scene->background->height)].color_value;
			union argb_pixel bottom;
			bottom.color_value = screen[i + j * width];

			ptr[i + j * width].color.r = (char)(((unsigned int)top.color.r * (unsigned int)top.color.r + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / 0xff);
			ptr[i + j * width].color.g = (char)(((unsigned int)top.color.g * (unsigned int)top.color.g + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / 0xff);
			ptr[i + j * width].color.b = (char)(((unsigned int)top.color.b * (unsigned int)top.color.b + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / 0xff);


		}
	}

	struct menu_item* items[256];

	for (int i = 0; i < scene->menu_items_count; i++) items[i] = &scene->menu_items[i];

	qsort(items, scene->menu_items_count, sizeof(void*), compare);

	for (int i = 0; i < scene->menu_items_count; i++) {
		switch (items[i]->menu_item_type) {

		case MENU_ITEM_LABEL: {

		}

		case MENU_ITEM_IMAGE: {

		}

		case MENU_ITEM_BUTTON: {

		}

		case MENU_ITEM_SLIDER: {

		}

		case MENU_ITEM_TEXT_FIELD: {

		}

		default:
			break;
		}
	}

}
