#include <stdlib.h>
#include <stdio.h>

#include "menu.h"
#include "general/utils.h"


void add_menu_label(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, struct gui_character* text, char text_alignment) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_LABEL;
	scene->menu_items[scene->menu_items_count].z = z;
	scene->menu_items[scene->menu_items_count].items.label = (struct menu_label){ x, y, alignment_x, alignment_y, text, text_alignment };
	scene->menu_items_count++;
}

void add_menu_image(struct menu_scene* scene, int z, int x, int y, char alignment_x, char alignment_y, char image_alignment_x, char image_alignment_y, struct argb_image* image, int image_scalar) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_IMAGE;
	scene->menu_items[scene->menu_items_count].z = z;
	scene->menu_items[scene->menu_items_count].items.image = (struct menu_image){ x, y, alignment_x, alignment_y, image_alignment_x, image_alignment_y, image, image_scalar };
	scene->menu_items_count++;
}

void add_menu_button(struct menu_scene* scene, int z, bool* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, struct argb_image* texture_enabled, struct argb_image* texture_disabled, bool* enabled) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_BUTTON;
	scene->menu_items[scene->menu_items_count].z = z;
	scene->menu_items[scene->menu_items_count].items.button = (struct menu_button){ state, x_min, y_min, x_max, y_max, alignment_x, alignment_y, texture_enabled, texture_disabled, enabled };
	scene->menu_items_count++;
}

void add_menu_slider(struct menu_scene* scene, int z, float* state, int x_min, int y_min, int x_max, int y_max, char alignment_x, char alignment_y, struct argb_image* texture_background, struct argb_image* texture_slider, int slider_thickness) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_SLIDER;
	scene->menu_items[scene->menu_items_count].z = z;
	scene->menu_items[scene->menu_items_count].items.slider = (struct menu_slider){ state, x_min, y_min, x_max, y_max, alignment_x, alignment_y, texture_background, texture_slider, slider_thickness };
	scene->menu_items_count++;
}

void add_menu_text_field(struct menu_scene* scene, int z, char* buffer, int x_min, int x_max, int y, char alignment_x, char alignment_y, char text_alignment, bool* selected, struct char_font* font) {
	scene->menu_items[scene->menu_items_count].menu_item_type = MENU_ITEM_TEXT_FIELD;
	scene->menu_items[scene->menu_items_count].z = z;
	scene->menu_items[scene->menu_items_count].items.text_field = (struct menu_text_field){ buffer, x_min, x_max, y, alignment_x, alignment_y, text_alignment, selected, font };
	scene->menu_items_count++;
}

int compare(const void* a, const void* b) {
	return ((*(struct menu_item**)a)->z - (*(struct menu_item**)b)->z);
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

void menu_scene_frame(struct menu_scene* scene, int scale, unsigned int* screen, int width, int height, int mouse_x, int mouse_y, char click) {

	union argb_pixel* screen_argb = (union argb_pixel*)screen;

	struct menu_item* items[128];

	for (int i = 0; i < scene->menu_items_count; i++) items[i] = &scene->menu_items[i];

	qsort(items, scene->menu_items_count, sizeof(void*), compare);

	

	for (int i = 0; i < scene->menu_items_count; i++) {
		switch (items[i]->menu_item_type) {

		case MENU_ITEM_LABEL: {
			if(items[i]->items.label.text == NULL) break;
			struct gui_character text_copy[64];
			int j = 0;
			for (; items[i]->items.label.text[j].value != '\0' && j < 63; j++) {
				text_copy[j].color = 0xff3b3b3b;
				text_copy[j].font = items[i]->items.label.text[j].font;
				text_copy[j].value = items[i]->items.label.text[j].value;
				text_copy[j].size = items[i]->items.label.text[j].size;
			}

			text_copy[j].color = 0xff3b3b3b;
			text_copy[j].font = items[i]->items.label.text[j].font;
			text_copy[j].value = items[i]->items.label.text[j].value;
			text_copy[j].size = items[i]->items.label.text[j].size;

			

			print_gui_string(
				text_copy,
				scale,
				menu_x(items[i]->items.label.x + items[i]->items.label.text[0].size/3 + 1, items[i]->items.label.alignment_x, scale, width),
				menu_y(items[i]->items.label.y + items[i]->items.label.text[0].size/3 + 1, items[i]->items.label.alignment_y, scale, height),
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

			int x_min_actually = menu_x(image->x, image->alignment_x, scale, width) - (image->image_alignment_x == ALIGNMENT_LEFT ? 0 : (image->image_alignment_x == ALIGNMENT_MIDDLE ? image->image->width * scale * image->image_scalar / 2 : image->image->width * scale * image->image_scalar));
			int y_min_actually = menu_x(image->x, image->alignment_x, scale, width) - (image->image_alignment_y == ALIGNMENT_TOP ? 0 : (image->image_alignment_y == ALIGNMENT_MIDDLE ? image->image->height * scale * image->image_scalar / 2 : image->image->height * scale * image->image_scalar));

			int x_min = clamp_int(x_min_actually, 0, width);
			int x_max = clamp_int(x_min + image->image->width * scale * image->image_scalar, 0, width);
			int y_min = clamp_int(y_min_actually, 0, height);
			int y_max = clamp_int(y_min + image->image->height * scale * image->image_scalar, 0, height);

			for (int x = x_min; x < x_max; x++) {
				for (int y = y_min; y < y_max; y++) {
					screen[x + y * width] = image->image->pixels[(x - x_min_actually) / (scale * image->image_scalar) + image->image->width * ((y - y_min_actually) / (scale * image->image_scalar))].color_value;
				}
			}

			break;
		}

		case MENU_ITEM_BUTTON: {

			struct menu_button* button = &(items[i]->items.button);

			int x_min_actually = menu_x(button->x_min, button->alignment_x, scale, width);
			int x_max_actually = menu_x(button->x_max, button->alignment_x, scale, width);
			int y_min_actually = menu_y(button->y_min, button->alignment_y, scale, height);
			int y_max_actually = menu_y(button->y_max, button->alignment_y, scale, height);

			int x_min = clamp_int(x_min_actually, 0, width);
			int x_max = clamp_int(x_max_actually, 0, width);
			int y_min = clamp_int(y_min_actually, 0, height);
			int y_max = clamp_int(y_max_actually, 0, height);


			unsigned int hover_color = 0xff000000;
			if (mouse_x >= x_min && mouse_x < x_max && mouse_y >= y_min && mouse_y < y_max && *(button->enabled)) { hover_color = 0xffffffff; if (click == 0b11) *button->state = true; }

			x_min = (x_min < 0 ? 0 : x_min);

			unsigned int default_enabled_color = 0xff7f7f7f;
			unsigned int default_disabled_color = 0xff424242;
			struct argb_image default_enabled_image = { 1, 1, (union argb_pixel*) &default_enabled_color };
			struct argb_image default_disabled_image = { 1, 1, (union argb_pixel*) &default_disabled_color };
			struct argb_image* texture = (*(button->enabled) ? (button->texture_enabled != NULL ? button->texture_enabled : &default_enabled_image) : (button->texture_disabled != NULL ? button->texture_disabled : &default_disabled_image));

			if (*(button->enabled)) {
				for (int x = x_min; x < x_max; x++) {
					for (int y = y_min; y < y_max; y++) {

						if (x >= x_min_actually + 2 * scale && x < x_max_actually - 2 * scale && y >= y_min_actually + 2 * scale && y < y_max_actually - 3 * scale) {
							screen[x + y * width] = texture->pixels[((x - x_min_actually) / scale) % texture->width + texture->width * (((y - y_min_actually) / scale) % texture->height)].color_value;
						}
						else if (x < x_min_actually + scale || x >= x_max_actually - scale || y < y_min_actually + scale || y >= y_max_actually - scale) screen[x + width * y] = hover_color;
						else {
							if (x < x_min_actually + 2 * scale) screen[x + width * y] = 0xffafafaf;
							else if (y < y_min_actually + 2 * scale) screen[x + width * y] = 0xffafafaf;
							if (x >= x_max_actually - 2 * scale) screen[x + width * y] = 0xff5c5c5c;
							else if (y >= y_max_actually - 3 * scale) screen[x + width * y] = 0xff5c5c5c;
						}
					}
				}
			}
			else {
				for (int x = x_min; x < x_max; x++) {
					for (int y = y_min; y < y_max; y++) {

						if (x >= x_min_actually + scale && x < x_max_actually - scale && y >= y_min_actually + scale && y < y_max_actually - scale) {
							screen[x + y * width] = texture->pixels[((x - x_min_actually) / scale) % texture->width + texture->width * (((y - y_min_actually) / scale) % texture->height)].color_value;
						}
						else screen[x + width * y] = 0xff000000;
					}
				}
			}

			break;
		}

		case MENU_ITEM_SLIDER: {
			struct menu_slider* slider = &(items[i]->items.slider);

			int x_min_actually = menu_x(slider->x_min, slider->alignment_x, scale, width);
			int x_max_actually = menu_x(slider->x_max, slider->alignment_x, scale, width);
			int y_min_actually = menu_y(slider->y_min, slider->alignment_y, scale, height);
			int y_max_actually = menu_y(slider->y_max, slider->alignment_y, scale, height);

			int x_min = clamp_int(x_min_actually, 0, width);
			int x_max = clamp_int(x_max_actually, 0, width);
			int y_min = clamp_int(y_min_actually, 0, height);
			int y_max = clamp_int(y_max_actually, 0, height);
			
			unsigned int default_button_color = 0xff888888;
			unsigned int default_color = 0xff888888;
			
			for (int x = x_min; x < x_max; x++) {
				for (int y = y_min; y < y_max; y++) {

					if (x >= x_min_actually + scale && x < x_max_actually - scale && y >= y_min_actually + scale && y < y_max_actually - scale) {
						screen[x + y * width] = slider->texture_background->pixels[((x - x_min_actually) / scale) % slider->texture_background->width + slider->texture_background->width * (((y - y_min_actually) / scale) % slider->texture_background->height)].color_value;
					}
					else screen[x + width * y] = 0xff000000;
				}
			}

			unsigned int hover_color = 0xff000000;

			

			int x_min_actually_slider = menu_x(slider->x_min + (int)(clamp_float(*slider->state, 0.f, 1.f) * ((float)slider->x_max - (float)slider->x_min - (float)slider->slider_thickness)), slider->alignment_x, scale, width);
			int x_max_actually_slider = menu_x(slider->x_min + slider->slider_thickness + (int)(clamp_float(*slider->state, 0.f, 1.f) * ((float)slider->x_max - (float)slider->x_min - (float)slider->slider_thickness)), slider->alignment_x, scale, width);

			int x_min_slider = clamp_int(x_min_actually_slider, 0, width);
			int x_max_slider = clamp_int(x_max_actually_slider, 0, width);
	
			if (mouse_x >= x_min && mouse_x < x_max && mouse_y >= y_min && mouse_y < y_max) {
				hover_color = 0xffffffff;
				if (click) {
					*slider->state = clamp_float(((float)mouse_x - (float)x_min_actually - (float)slider->slider_thickness * (float)scale / 2) / ((float)x_max_actually - (float)x_min_actually - (float)slider->slider_thickness * (float)scale), 0.f, 1.f);
				}
			}

			for (int x = x_min_slider; x < x_max_slider; x++) {
				for (int y = y_min; y < y_max; y++) {

					if (x >= x_min_actually_slider + 2 * scale && x < x_max_actually_slider - 2 * scale && y >= y_min_actually + 2 * scale && y < y_max_actually - 3 * scale) {
						screen[x + y * width] = slider->texture_slider->pixels[((x - x_min_actually_slider) / scale) % slider->texture_slider->width + slider->texture_slider->width * (((y - y_min_actually) / scale) % slider->texture_slider->height)].color_value;
					}
					else if (x < x_min_actually_slider + scale || x >= x_max_actually_slider - scale || y < y_min_actually + scale || y >= y_max_actually - scale) screen[x + width * y] = hover_color;
					else {
						if (x < x_min_actually_slider + 2 * scale) screen[x + width * y] = 0xffafafaf;
						else if (y < y_min_actually + 2 * scale) screen[x + width * y] = 0xffafafaf;
						if (x >= x_max_actually_slider - 2 * scale) screen[x + width * y] = 0xff5c5c5c;
						else if (y >= y_max_actually - 3 * scale) screen[x + width * y] = 0xff5c5c5c;
					}
				}
			}

			break;
		}

		case MENU_ITEM_TEXT_FIELD: {

			struct menu_text_field* text_field = &(items[i]->items.text_field);

			int x_min_actually = menu_x(text_field->x_min, text_field->alignment_x, scale, width);
			int x_max_actually = menu_x(text_field->x_max, text_field->alignment_x, scale, width);
			int y_min_actually = menu_y(text_field->y - 10, text_field->alignment_y, scale, height);
			int y_max_actually = menu_y(text_field->y + 10, text_field->alignment_y, scale, height);

			int x_min = clamp_int(x_min_actually, 0, width);
			int x_max = clamp_int(x_max_actually, 0, width);
			int y_min = clamp_int(y_min_actually, 0, height);
			int y_max = clamp_int(y_max_actually, 0, height);

			if (click) {
				if (mouse_x >= x_min && mouse_x < x_max && mouse_y >= y_min && mouse_y < y_max) *text_field->selected = true;
				else *text_field->selected = false;
			}
			
			int frame_color = 0xffa0a0a0;

			if (*text_field->selected) frame_color = 0xffffffff;

			for (int x = x_min; x < x_max; x++) {
				for (int y = y_min; y < y_max; y++) {

					if (x >= x_min_actually + scale && x < x_max_actually - scale && y >= y_min_actually + scale && y < y_max_actually - scale) {
						screen[x + y * width] = 0xff000000;
					}
					else screen[x + width * y] = frame_color;
				}
			}

			if (text_field->buffer[0] != '\0') {
				struct gui_character* text = convert_string_to_gui_string(text_field->font, text_field->buffer, 1, 0xffffffff );
				struct gui_character* text_shadow = convert_string_to_gui_string(text_field->font, text_field->buffer, 1, 0xff3b3b3b);

				int x = 0;

				if (text_field->text_alignment == ALIGNMENT_LEFT) x = text_field->x_min + 3;
				else if (text_field->text_alignment == ALIGNMENT_MIDDLE) x = (text_field->x_min + text_field->x_min) / 2;
				else if (text_field->text_alignment == ALIGNMENT_RIGHT) x = text_field->x_max - 3;

				print_gui_string(
					text_shadow,
					scale,
					menu_x(x + 1, text_field->alignment_x, scale, width),
					menu_y(text_field->y + 1, text_field->alignment_y, scale, height),
					text_field->text_alignment,
					screen,
					width,
					height
				);

				print_gui_string(
					text,
					scale,
					menu_x(x, text_field->alignment_x, scale, width),
					menu_y(text_field->y, text_field->alignment_y, scale, height),
					text_field->text_alignment,
					screen,
					width,
					height
				);

				free(text);
				free(text_shadow);

			}


			break;
		}

		default:
			break;
		}
	}

}
