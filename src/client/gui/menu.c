#include <stdlib.h>
#include <stdio.h>

#include "menu.h"



void add_menu_label(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, struct gui_character* text, char text_alignment) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_LABEL;
	scene->menu_items[scene->menu_items_count].items.label = (struct menu_label){ z, x, y, alignment_x, alignment_y, {0}, text_alignment };
	scene->menu_items[scene->menu_items_count].items.label.text[0] = text[0];
	for (int i = 1; text[i-1].value != '\0'; i++) scene->menu_items[scene->menu_items_count].items.label.text[i] = text[i];
	scene->menu_items_count++;
}

void add_menu_image(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, char image_alignment_x, char image_alignment_y, struct argb_image* image, int image_scalar) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_IMAGE;
	scene->menu_items[scene->menu_items_count].items.image = (struct menu_image){ z, x, y, alignment_x, alignment_y, image_alignment_x, image_alignment_y, image, image_scalar };
	scene->menu_items_count++;
}

void add_menu_button(struct menu_scene* scene, int z, bool* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, struct argb_image* texture_enabled, struct argb_image* texture_disabled, bool* enabled) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_BUTTON;
	scene->menu_items[scene->menu_items_count].items.button = (struct menu_button){ z, state, x_min, y_min, x_max, y_max, alignment_x, alignment_y, texture_enabled, texture_disabled, enabled };
	scene->menu_items_count++;
}

void add_menu_text_slider(struct menu_scene* scene, int z, float* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, unsigned int color) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_SLIDER;
	scene->menu_items[scene->menu_items_count].items.slider = (struct menu_slider){ z, state, x_min, y_min, x_max, y_max, alignment_x, alignment_y, color};
	scene->menu_items_count++;
}

void add_menu_text_field(struct menu_scene* scene, int z, char* buffer, int buffer_size, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_TEXT_FIELD;
	scene->menu_items[scene->menu_items_count].items.text_field = (struct menu_text_field){ z, buffer, buffer_size, 0, x_min, y_min, x_max, y_max, alignment_x, alignment_y };
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

void render_menu_scene(struct menu_scene* scene, int scale, unsigned int* screen, int width, int height, int mouse_x, int mouse_y) {

	union argb_pixel* screen_argb = screen;

	struct menu_item* items[128];

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
			break;
		}

		case MENU_ITEM_IMAGE: {
			struct menu_image* image = &(items[i]->items.image);

			int x_start = menu_x(image->x, image->alignment_x, scale, width) - (image->image_alignment_x == ALIGNMENT_LEFT ? 0 : (image->image_alignment_x == ALIGNMENT_MIDDLE ? image->image->width * scale * image->image_scalar / 2 : image->image->width * scale * image->image_scalar));
			int y_start = menu_x(image->x, image->alignment_x, scale, width) - (image->image_alignment_y == ALIGNMENT_TOP ? 0 : (image->image_alignment_y == ALIGNMENT_MIDDLE ? image->image->height * scale * image->image_scalar / 2 : image->image->height * scale * image->image_scalar));

			for (int x = (x_start < 0 ? 0 : x_start); x < image->image->width * scale * image->image_scalar && x < width; x++) {
				for (int y = (y_start < 0 ? 0 : y_start); y < image->image->height * scale * image->image_scalar && y < height; y++) {
					screen[x + y * width] = image->image->pixels[(x - x_start) / (scale * image->image_scalar) + image->image->width * ((y - y_start) / (scale * image->image_scalar))].color_value;
				}
			}

			break;
		}

		case MENU_ITEM_BUTTON: {

			struct menu_button* button = &(items[i]->items.button);

			int x_min = menu_x(button->x_min, button->alignment_x, scale, width);
			int x_max = menu_x(button->x_max, button->alignment_x, scale, width);
			int y_min = menu_y(button->y_min, button->alignment_y, scale, height);
			int y_max = menu_y(button->y_max, button->alignment_y, scale, height);

		unsigned int press_color = 0xff000000;

			if (mouse_x >= x_min && mouse_x < x_max && mouse_y >= y_min && mouse_y < y_max && *(button->enabled)) press_color = 0xffffffff;

			unsigned int default_color = 0xff888888;
			struct argb_image default_image = { 1, 1, &default_color };

			struct argb_image* texture = (*(button->enabled) ? (button->texture_enabled != NULL ? button->texture_enabled : &default_image) : (button->texture_disabled != NULL ? button->texture_disabled : &default_image));

			for (int x = (x_min < 0 ? 0 : x_min); x < x_max && x < width; x++) {
				for (int y = (y_min < 0 ? 0 : y_min); y < y_max && y < height; y++) {
						
					if (x >= x_min + 2 * scale && x < x_max - 2 * scale && y >= y_min + 2 * scale && y < y_max - 3 * scale) {
						union argb_pixel top;
						top.color_value = texture->pixels[((x) / scale) % texture->width + texture->width * (((y) / scale) % texture->height)].color_value;
						union argb_pixel bottom;
						bottom.color_value = screen[x + y * width];

						screen_argb[x + y * width].color.r = (char)(((unsigned int)top.color.r * (unsigned int)top.color.a + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / 0xff);
						screen_argb[x + y * width].color.g = (char)(((unsigned int)top.color.g * (unsigned int)top.color.a + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / 0xff);
						screen_argb[x + y * width].color.b = (char)(((unsigned int)top.color.b * (unsigned int)top.color.a + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / 0xff);
					}

					else if (x < x_min + scale || x >= x_max - scale || y < y_min + scale || y >= y_max - scale) screen[x + width * y] = press_color;

					else {
						if (x < x_min + 2 * scale) screen[x + width * y] = 0xffafafaf;
						else if (y < y_min + 2 * scale) screen[x + width * y] = 0xffafafaf;
						if (x >= x_max - 2 * scale) screen[x + width * y] = 0xff5c5c5c;
						else if (y >= y_max - 3 * scale) screen[x + width * y] = 0xff5c5c5c;
					}	
					
					
				}
			}

			break;
		}

		case MENU_ITEM_SLIDER: {
			break;
		}

		case MENU_ITEM_TEXT_FIELD: {
			break;
		}

		default:
			break;
		}
	}

}
