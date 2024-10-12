#include "menu.h"

#include <stdlib.h>
#include <malloc.h>

int compare(const void* a, const void* b) {
	return ((*(int**)a)[1] - (*(int**)b)[1]);
}

int menu_x(int menu_x, int menu_alignment, int menu_width, int scale) {

	int x = menu_x * scale;

	if (menu_alignment == ALIGNMENT_LEFT) x += 0;
	else if (menu_alignment == ALIGNMENT_RIGHT) x += menu_width;
	else if (menu_alignment == ALIGNMENT_MIDDLE) x += menu_width / 2;

	return x;
}

int menu_y(int menu_y, int menu_alignment, int menu_height, int scale) {

	int y = menu_y * scale;

	if (menu_alignment == ALIGNMENT_TOP) y += 0;
	else if (menu_alignment == ALIGNMENT_BOTTOM) y += menu_height;
	else if (menu_alignment == ALIGNMENT_MIDDLE) y += menu_height / 2;

	return y;
}

void menu_frame(struct gui_menu* menu, unsigned int* screen, int width, int height, int scale, const void** resource_map, int mouse_click, int mouse_x, int mouse_y) {

	union argb_pixel* screen_argb = (union argb_pixel*)screen;

	int** ordered_menu_items = alloca(sizeof(int*) * menu->items_count);;

	memcpy(ordered_menu_items, menu->items, sizeof(void*) * menu->items_count);

	qsort(ordered_menu_items, menu->items_count, sizeof(void*), compare);

	for (int i = 0; i < menu->items_count; i++) {
		switch (ordered_menu_items[i][0]) {

		case MENU_ITEM_LABEL: {

			struct menu_label* label = ordered_menu_items[i];
			
			int string_size = 1;
			for (; label->text[string_size - 1].value != '\0'; string_size++);

			struct pixel_char* text = alloca(sizeof(struct pixel_char) * string_size);

			memcpy(text, label->text, sizeof(struct pixel_char) * string_size);

			if (label->selectable) {

				int select_index = pixel_char_get_hover_index(
					text,
					label->text_size * scale,
					2,
					menu_x(label->x, label->alignment_x, width, scale),
					menu_y(label->y, label->alignment_y, height, scale),
					label->text_alignment_x,
					label->text_alignment_y,
					label->max_width,
					label->max_rows,
					resource_map,
					mouse_x,
					mouse_y
				);


				if (mouse_click == 0b11) {

					if (menu->select_label == i && menu->select_begin == select_index && menu->selecting == 1) {

						for (menu->select_end = select_index; ; menu->select_end++) if (
							(text[menu->select_end].value < 'a' || text[menu->select_end].value > 'z') &&
							(text[menu->select_end].value < 'A' || text[menu->select_end].value > 'Z') &&
							(text[menu->select_end].value < '0' || text[menu->select_end].value > '9')
							) break;

						if (menu->select_end != select_index) menu->select_end--;

						for (menu->select_begin = select_index; menu->select_begin > 0; menu->select_begin--) if (
							(text[menu->select_begin - 1].value < 'a' || text[menu->select_begin - 1].value > 'z') &&
							(text[menu->select_begin - 1].value < 'A' || text[menu->select_begin - 1].value > 'Z') &&
							(text[menu->select_begin - 1].value < '0' || text[menu->select_begin - 1].value > '9')
							) break;

						menu->selecting = 0;
					}
					else {
						menu->select_label = i;
						menu->select_begin = select_index;
						menu->select_end = -1;
						menu->selecting = 1;
					}

				}

				else if (mouse_click && menu->select_label == i && select_index != -1 && (menu->select_begin != select_index || menu->select_end != -1) && menu->selecting == 1) {
					menu->select_end = select_index;
				}

				if (menu->select_begin >= 0 && menu->select_end >= 0) {
					for (int j = min(menu->select_begin, menu->select_end); j <= max(menu->select_begin, menu->select_end); j++) {
						text[j].color = 0xffffff00;
						text[j].masks |= PIXEL_CHAR_BACKGROUND_MASK;
					}
				}
			}

			pixel_char_print_string(
				text, 
				label->text_size * scale, 
				2, 
				menu_x(label->x, label->alignment_x, width, scale), 
				menu_y(label->y, label->alignment_y, height, scale), 
				label->text_alignment_x, 
				label->text_alignment_y, 
				label->max_width,
				label->max_rows,
				screen, 
				width, 
				height, 
				resource_map
			);

			break;
		}

		case MENU_ITEM_IMAGE: {

			struct menu_image* image = ordered_menu_items[i];

			argb_image_draw(
				resource_map[image->image], 
				image->x, 
				image->y, 
				image->image_alignment_x, 
				image->image_alignment_y, 
				0, 
				0, 
				screen, 
				width, 
				height, 
				image->image_scalar
			);

			break;
		}

		/*case MENU_ITEM_SLIDER: {
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
		}*/

		default:
			break;
		}
	}

}