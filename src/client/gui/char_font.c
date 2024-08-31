#include "stdlib.h"
#include "string.h"

#include "char_font.h"

#include "client/platform.h"

#include "general/argb_image.h"
#include "general/resource_manager.h"
#include "general/utils.h"

struct char_font* load_char_font(char* src) {

	char* bytes = parse_file(src);
	if (bytes == NULL) return NULL;

	struct char_font* font = malloc(sizeof(struct char_font));
	if (font == NULL) { free(bytes);  return NULL; }

	for (int i = 0; i < sizeof(struct char_font); i++) ((char*)font)[i] = bytes[i];
	
	return font;
}

void print_char(struct char_font* font, char c, int text_size, unsigned int color, int x, int y, unsigned int* screen, int width, int height) {

	for (int i = x; i < width && i < x + font->char_font_entries[c].width * text_size; i++) {
		for (int j = y; j < height && j < y + 8 * text_size; j++) {

			if (font->char_font_entries[c].layout & (1LL << ((i - x) / text_size) + 8 * ((j - y) / text_size)) && i >= 0 && j >= 0) {

				union argb_pixel top;
				top.color_value = color;
				union argb_pixel bottom;
				bottom.color_value = screen[i + j * width];

				((union argb_pixel*)screen)[i + j * width].color.r = (char)(((unsigned int)top.color.r * (unsigned int)top.color.a + ((unsigned int)0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / ((unsigned int)0xff));
				((union argb_pixel*)screen)[i + j * width].color.g = (char)(((unsigned int)top.color.g * (unsigned int)top.color.a + ((unsigned int)0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / ((unsigned int)0xff));
				((union argb_pixel*)screen)[i + j * width].color.b = (char)(((unsigned int)top.color.b * (unsigned int)top.color.a + ((unsigned int)0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / ((unsigned int)0xff));
			}
		}
	}


}

void print_string(struct char_font* font, char* str, int text_size, unsigned int color, int x, int y, unsigned int* screen, int width, int height) {
	
	int x_pos = x;

	for (int i = 0; str[i] != '\0'; i++) {
		print_char(font, str[i], text_size, color, x_pos, y, screen, width, height);
		x_pos += text_size + text_size * clamp_int(font->char_font_entries[str[i]].width, 0, 8);
	}
}

void drop_font(struct char_font* font) {
	free(font);
}

struct gui_character* convert_string_to_gui_string(struct char_font* font, char* str) {
	int length = 0;
	for (; str[length] != '\0'; length++);
	length++;
	struct gui_character* gui_str = malloc(length * sizeof(struct gui_character));
	if (gui_str == NULL) { return NULL; }


	for (int i = 0; i < length; i++) {
		gui_str[i].color = 0xffffffff;
		gui_str[i].font = font;
		gui_str[i].size = 1;
		gui_str[i].value = str[i];
	}

	return gui_str;

}

void print_gui_string( struct gui_character* str, int scale, int x, int y, char alignment, unsigned int* screen, int width, int height) {

	int x_pos = 0;

	int text_width = gui_text_width(str);

	if (alignment == ALIGNMENT_LEFT) x_pos = x;
	else if (alignment == ALIGNMENT_MIDDLE) x_pos = x - text_width * scale / 2;
	else if (alignment == ALIGNMENT_RIGHT) x_pos = x - text_width * scale;

	for (int i = 0; str[i].value != '\0'; i++) {
		print_char(str[i].font, str[i].value, scale * str[i].size, str[i].color, x_pos, y - 4 * str[i].size * scale, screen, width, height);
		x_pos += scale * str[i].size + scale * str[i].size * clamp_int(str[i].font->char_font_entries[str[i].value].width, 0, 8);
	}
}

int gui_text_width(struct gui_character* str) {
	int total_width = 0;
	int i = 0;
	for (; str[i].value != '\0'; i++) total_width += str[i].size * (1 + str[i].font->char_font_entries[str[i].value].width);
	if(i != 0) total_width -= str[i].size;
	return total_width;
}