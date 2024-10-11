#include "test/menu.h"

#include <stdlib.h>

#include "general/utils.h"

int compare(const void* a, const void* b) {
	return ((*(struct menu_item**)a)->z - (*(struct menu_item**)b)->z);
}

int menu_object_x(int menu_x, int menu_alignment, int menu_width, int object_alignment, int object_width, int scale) {

	int x = menu_x * scale;

	if (menu_alignment == MENU_ALIGNMENT_LEFT) x += 0;
	else if (menu_alignment == MENU_ALIGNMENT_RIGHT) x += menu_width;
	else if (menu_alignment == MENU_ALIGNMENT_MIDDLE) x += menu_width / 2;

	if (object_alignment == MENU_ALIGNMENT_LEFT) x += 0;
	else if (object_alignment == MENU_ALIGNMENT_RIGHT) x -= object_width;
	else if (object_alignment == MENU_ALIGNMENT_MIDDLE) x -= object_width / 2;

	return x;
}

int menu_object_y(int menu_y, int menu_alignment, int menu_height, int object_alignment, int object_height, int scale) {

	int y = menu_y * scale;

	if (menu_alignment == MENU_ALIGNMENT_LEFT) y += 0;
	else if (menu_alignment == MENU_ALIGNMENT_BOTTOM) y += menu_height * scale;
	else if (menu_alignment == MENU_ALIGNMENT_TOP) y += menu_height * scale / 2;

	if (object_alignment == MENU_ALIGNMENT_LEFT) y += 0;
	else if (object_alignment == MENU_ALIGNMENT_BOTTOM) y -= object_height * scale;
	else if (object_alignment == MENU_ALIGNMENT_TOP) y -= object_height * scale / 2;

	return y;
}

void menu_frame(struct menu_item** menu_items, int menu_items_count, unsigned int* screen, int width, int height, int scale, void** resource_map) {

	union argb_pixel* screen_argb = (union argb_pixel*)screen;

	qsort(menu_items, menu_items_count, sizeof(void*), compare);

	for (int i = 0; i < menu_items_count; i++) {
		switch (menu_items[i]->menu_item_type) {

		case MENU_ITEM_LABEL: {
			if (menu_items[i]->items.label.text == NULL) break;

			struct menu_label* label = &(menu_items[i]->items.label);

			int lines = 1;
			for (int j = 0; label->text[j].value != '\0'; j++) if (label->text[j].value == '\n') lines++;

			int y_min = menu_object_x(label->y, label->alignment_x, width, label->text_alignment_x, 8 * lines, scale);

			int text_i;

			for (int j = 0; j < lines; j++) {
				if (label->text[text_i].value == '\n') continue;
				if (label->text[text_i].value == '\0') break;

				int line_width = 0;

				line_width += ((struct pixel_font**)resource_map)[label->text[text_i].underline_cursive_font & FONT_MASK]->char_font_entries[label->text[text_i].value].width;

				for (int k = text_i + 1; label->text[k].value != '\n' && label->text[k].value != '\0'; k++) {
					line_width += ((struct pixel_font**)resource_map)[label->text[text_i].underline_cursive_font & FONT_MASK]->char_font_entries[label->text[text_i].value].width;
				}

				int x_min = menu_object_x(label->x, label->alignment_x, width, label->text_alignment_x, line_width, scale);

				int x_cursor;

				for (; label->text[text_i].value != '\n' && label->text[text_i].value != '\0'; text_i++) {
				
				}


				if (label->text[text_i].value == '\0') break;

				y_min += 8;
				text_i++;
			}

			

			break;
		}

		case MENU_ITEM_IMAGE: {
			struct menu_image* image = &(menu_items[i]->items.image);

			struct argb_image* image_resource = (struct argb_image*)(resource_map[image->image]);

			int x_min_actually = menu_object_x(image->x, image->alignment_x, width, image->image_alignment_x, image_resource->width * image->image_scalar, scale);
			int y_min_actually = menu_object_x(image->y, image->alignment_y, height, image->image_alignment_y, image_resource->height * image->image_scalar, scale);

			int x_min = clamp_int(x_min_actually, 0, width);
			int x_max = clamp_int(x_min + image_resource->width * scale * image->image_scalar, 0, width);
			int y_min = clamp_int(y_min_actually, 0, height);
			int y_max = clamp_int(y_min + image_resource->height * scale * image->image_scalar, 0, height);

			for (int x = x_min; x < x_max; x++) {
				for (int y = y_min; y < y_max; y++) {
					union argb_pixel top;
					top.color_value = image_resource->pixels[(x - x_min_actually) / (scale * image->image_scalar) + image_resource->width * ((y - y_min_actually) / (scale * image->image_scalar))].color_value;

					screen_argb[x + y * width].color.r = (unsigned char)(((unsigned int)top.color.r * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)screen_argb[x + y * width].color.r) / 255);
					screen_argb[x + y * width].color.g = (unsigned char)(((unsigned int)top.color.g * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)screen_argb[x + y * width].color.g) / 255);
					screen_argb[x + y * width].color.b = (unsigned char)(((unsigned int)top.color.b * (unsigned int)top.color.a + (255 - (unsigned int)top.color.a) * (unsigned int)screen_argb[x + y * width].color.b) / 255);

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
			struct argb_image default_enabled_image = { 1, 1, (union argb_pixel*)&default_enabled_color };
			struct argb_image default_disabled_image = { 1, 1, (union argb_pixel*)&default_disabled_color };
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

			if (*text_field->field_visible) {

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
			}

			if (text_field->buffer[0] != '\0') {
				struct gui_character* text = convert_string_to_gui_string(text_field->font, text_field->buffer, 1, 0xffffffff);
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