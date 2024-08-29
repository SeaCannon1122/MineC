#include "stdlib.h"
#include "string.h"

#include "general/resource_manager.h"
#include "general/argb_image.h"

struct char_font {
	struct {
		char width;
		char state[35];
	} char_font_entries[256];
};

struct char_font* load_char_font(char* src) {

	struct char_font* font = malloc(sizeof(struct char_font));
	char* bytes = parse_file(src);

	for (int i = 0, bytes[i] != '\0' && i < 36 * 256; i++) ((char*)font)[i] = bytes[i];
}

void print_char(struct char_font* font, char c, int font_width, unsigned int color, int x, int y, unsigned int* screen, int width, int height) {

	for (int i = x; i < width && i < x + font->char_font_entries[c].width * font_width) {
		for (int j = y; j < height && j < x + 7 * font_width) {

			if (font->char_font_entries[c].state[5 * (i - x) / font_width + (j - x) / font_width]) {

				union argb_pixel top;
				top.color_value = color;
				union argb_pixel bottom;
				bottom.color_value = screen[i + j * width];

				ptr[i + j * width].color.r = (char)(((unsigned int)top.color.r * (unsigned int)top.color.r + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.r) / 0xff);
				ptr[i + j * width].color.g = (char)(((unsigned int)top.color.g * (unsigned int)top.color.g + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.g) / 0xff);
				ptr[i + j * width].color.b = (char)(((unsigned int)top.color.b * (unsigned int)top.color.b + (0xff - (unsigned int)top.color.a) * (unsigned int)bottom.color.b) / 0xff);
			}
		}
	}


}

void drop_font(struct char_font* font) {
	free(font);
}