#include "pixel_char.h"

void pixel_char_print(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map) {
	struct pixel_font* font = resource_map[c->underline_cursive_font & PIXEL_CHAR_FONT_MASK];

	for (int j = 0; j < PIXEL_FONT_RESOULUTION * text_size / 2; j++) {

		int cursive_offset = (c->underline_cursive_font & PIXEL_CHAR_CURSIVE_MASK ? PIXEL_FONT_RESOULUTION * text_size / 4 - j / 2 - 1 : 0);

		for (int i = 0; i < (font->char_font_entries[c->value].width * text_size + 1) / 2; i++) {

			int x_bit_pos = i - text_size * PIXEL_FONT_RESOULUTION / 4;
			int y_bit_pos = j - text_size * PIXEL_FONT_RESOULUTION / 4;

			int bit_pos = ((x_bit_pos < 0 ? x_bit_pos + 1 : x_bit_pos) * 2 / text_size + PIXEL_FONT_RESOULUTION / 2) + PIXEL_FONT_RESOULUTION * ((y_bit_pos < 0 ? y_bit_pos + 1 : y_bit_pos) * 2 / text_size + PIXEL_FONT_RESOULUTION / 2);

			if (x_bit_pos < 0) bit_pos -= 1;
			if (y_bit_pos < 0) bit_pos -= PIXEL_FONT_RESOULUTION;

			if (i + cursive_offset + x >= 0 && j + y >= 0 && i + cursive_offset + x < width && j + y < height) {
				if (IF_BIT(font->char_font_entries[c->value].layout, bit_pos)) screen[(i + cursive_offset + x) + width * (j + y)] = c->color;
			}

		}
	}

	if (c->underline_cursive_font & PIXEL_CHAR_UNDERLINE_MASK) {
		for (int j = PIXEL_FONT_RESOULUTION * text_size / 2; j < (PIXEL_FONT_RESOULUTION + 2) * text_size / 2; j++) {

			int cursive_offset = (c->underline_cursive_font & PIXEL_CHAR_CURSIVE_MASK ? PIXEL_FONT_RESOULUTION * text_size / 4 - j / 2 - 1 : 0);

			for (int i = -(text_size + 1) / 2; i < (font->char_font_entries[c->value].width * text_size + 1) / 2 + text_size / 2; i++) {

				if (i + cursive_offset + x >= 0 && j + y >= 0 && i + cursive_offset + x < width && j + y < height) screen[(i + cursive_offset + x) + width * (j + y)] = c->color;

			}
		}
	}


}

void pixel_char_print_string(const struct pixel_char* RESTRICT c, int text_size, int x, int y, unsigned int* RESTRICT screen, int width, int height, const const void** RESTRICT resource_map) {

	if (c->value == '\0') return;

	pixel_char_print(c, text_size, x, y, screen, width, height, resource_map);

	int x_pos = x + (((struct pixel_font*)(resource_map[c[0].underline_cursive_font & PIXEL_CHAR_FONT_MASK]))->char_font_entries[c[0].value].width + 1) / 2 * text_size + (text_size + 1) / 2;

	for (int i = 1; c[i].value != '\0'; i++) {
		pixel_char_print(&c[i], text_size, x_pos, y, screen, width, height, resource_map);

		x_pos += (((struct pixel_font*)(resource_map[c[i].underline_cursive_font & PIXEL_CHAR_FONT_MASK]))->char_font_entries[c[i].value].width + 1) / 2 * text_size + (text_size + 1) / 2;
	}

}