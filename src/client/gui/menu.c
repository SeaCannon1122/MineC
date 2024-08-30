#include <stdlib.h>
#include <stdio.h>

#include "menu.h"


void add_menu_label(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, struct gui_character* text, char text_alignment) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_LABEL;
	scene->menu_items[scene->menu_items_count].items.label = (struct menu_label){ z, x, y, alignment_x, alignment_y, {0}, text_alignment };
	for (int i = 0; i < 64; i++) scene->menu_items[scene->menu_items_count].items.label.text[i] = text[i];
	scene->menu_items_count++;
}

void add_menu_image(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, struct arb_image* image) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_IMAGE;
	scene->menu_items[scene->menu_items_count].items.image = (struct menu_image){ z, x, y, alignment_x, alignment_y, image };
	scene->menu_items_count++;
}

void add_menu_button(struct menu_scene* scene, int z, bool* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, unsigned int color, struct arb_image* texture) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_BUTTON;
	scene->menu_items[scene->menu_items_count].items.button = (struct menu_button){ z, state, x_min, x_max, y_min, y_max, alignment_x, alignment_y, color, texture };
	scene->menu_items_count++;
}

void add_menu_text_slider(struct menu_scene* scene, int z, float* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, unsigned int color) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_SLIDER;
	scene->menu_items[scene->menu_items_count].items.slider = (struct menu_slider){ z, state, x_min, x_max, y_min, y_max, alignment_x, alignment_y, color};
	scene->menu_items_count++;
}

void add_menu_text_field(struct menu_scene* scene, int z, char* buffer, int buffer_size, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_TEXT_FIELD;
	scene->menu_items[scene->menu_items_count].items.text_field = (struct menu_text_field){ z, buffer, buffer_size, 0, x_min, x_max, y_min, y_max, alignment_x, alignment_y };
	scene->menu_items_count++;
}

int compare(const void* a, const void* b) {
	return ((*(struct menu_item**)a)->items.button.z - (*(struct menu_item**)b)->items.button.z);
}

int menu_x(int x, int alignment, int scale, int width) {
	if (alignment == ALIGNMENT_LEFT) return x * scale;
	else if (alignment == ALIGNMENT_RIGHT) return width + x * scale;
	else if (alignment == ALIGNMENT_MIDDLE) return (width + 1) / 2 + x * scale;
	else return 0;
}

int menu_y(int y, int alignment, int scale, int height) {
	if (alignment == ALIGNMENT_TOP) return y * scale;
	else if (alignment == ALIGNMENT_BOTTOM) return height + y * scale;
	else if (alignment == ALIGNMENT_MIDDLE) return (height + 1) / 2 + y * scale;
	else return 0;
}

void render_menu_scene(struct menu_scene* scene, int scale, unsigned int* screen, int width, int height) {

	if (scene->background != NULL) {
		union argb_pixel* ptr = screen;

		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {

				union argb_pixel top;
				top.color_value = scene->background->pixels[((int)(i / scene->background_scalar)) % scene->background->width + scene->background->width * (((int)(j / scene->background_scalar)) % scene->background->height)].color_value;
				union argb_pixel bottom;
				bottom.color_value = screen[i + j * width];

				ptr[i + j * width].color.r = (char)(((unsigned int)top.color.r * (unsigned int)top.color.r + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / 0xff);
				ptr[i + j * width].color.g = (char)(((unsigned int)top.color.g * (unsigned int)top.color.g + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / 0xff);
				ptr[i + j * width].color.b = (char)(((unsigned int)top.color.b * (unsigned int)top.color.b + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / 0xff);


			}
		}
	}

	struct menu_item* items[256];

	for (int i = 0; i < scene->menu_items_count; i++) items[i] = &scene->menu_items[i];

	qsort(items, scene->menu_items_count, sizeof(void*), compare);

	

	for (int i = 0; i < scene->menu_items_count; i++) {
		switch (items[i]->menu_item_type) {

		case MENU_ITEM_LABEL: {

			struct gui_character text_copy[64];
			for (int j = 0; j < 64; j++) {
				text_copy[j].color = 0xff3b3b3b;
				text_copy[j].font = items[i]->items.label.text[j].font;
				text_copy[j].value = items[i]->items.label.text[j].value;
				text_copy[j].size = items[i]->items.label.text[j].size;
			}

			print_gui_string(
				text_copy,
				scale,
				menu_x(items[i]->items.label.x + 1, items[i]->items.label.alignment_x, scale, width),
				menu_y(items[i]->items.label.y + 1, items[i]->items.label.alignment_y, scale, height),
				items[i]->items.label.text_alignment,
				screen,
				width,
				height
			);

			print_gui_string(
				items[i]->items.label.text, 
				scale, 
				menu_x(items[i]->items.label.x, items[i]->items.label.alignment_x, scale, width), 
				menu_y(items[i]->items.label.y, items[i]->items.label.alignment_y, scale, height), 
				items[i]->items.label.text_alignment,
				screen, 
				width, 
				height
			);

		}

		case MENU_ITEM_IMAGE: {

		}

		case MENU_ITEM_BUTTON: {
			int x_min = menu_x(items[i]->items.button.x_min, items[i]->items.button.alignment_x, scale, width);
			int x_max = menu_x(items[i]->items.button.x_max, items[i]->items.button.alignment_x, scale, width);
			int y_min = menu_y(items[i]->items.button.y_min, items[i]->items.button.alignment_y, scale, height);
			int y_max = menu_y(items[i]->items.button.y_max, items[i]->items.button.alignment_y, scale, height);

			for (int x = x_min; x < x_max; x++) {
				for (int y = y_min; y < y_max; y++) {
					if (x >= 0 && x < width && y >= 0 && y < height) {
						//screen[x + width * y] = 0xffff0000;
					}
				}
			}

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
