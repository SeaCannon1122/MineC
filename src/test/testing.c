#include "testing.h"

#include <stdio.h>

#include "general/platformlib/platform.h"
#include "menu.h"

void char_c(int window, int character) {
	int str[2] = { character,  0};
	 
	printf(str);
}

struct pixel_font_entry font = {
			5,
			{
				0b00110000, 0b00000000,
				0b00110000, 0b00000000,
				0b00110000, 0b00000000,
				0b00110000, 0b00000000,
				0b11001100, 0b00000000,
				0b11001100, 0b00000000,
				0b11111100, 0b00000000,
				0b11111100, 0b00000000,
				0b00000011, 0b00000011,
				0b00000011, 0b00000011,
				0b00000011, 0b00000011,
				0b00000011, 0b00000011,
				0b00000000, 0b00000000,
				0b00000000, 0b00000000,
				0b00000000, 0b00000000,
				0b00000000, 0b00000000,
			}
};

void* font_addr = &font;

struct gui_char a = {
	0xffffffff,
	0,
	0x00000000,
	NULL,
};

int testing_main() {

	show_console_window();

	int width = 500;
	int height = 300;

	int window = window_create(100, 100, width, height, "NAME");

	int cid = window_add_char_callback(window, char_c);

	width = window_get_width(window);
	height = window_get_width(window);

	unsigned int* pixels = malloc(sizeof(unsigned int) * height * width);

	while (get_key_state(KEY_ESCAPE) == 0 && window_is_active(window)) {

		int new_width = window_get_width(window);
		int new_height = window_get_width(window);

		if (width != new_width || height != new_height) {
			height = new_height;
			width = new_width;
			free(pixels);
			pixels = malloc(sizeof(unsigned int) * height * width);
			for (int i = 0; i < height * width; i++) pixels[i] = 0x12345;
		}

		gui_print_char(&a, 2, 100, 100, pixels, width, height, &font_addr);

		struct point2d_int p = window_get_mouse_cursor_position(window);

		if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height && get_key_state(KEY_MOUSE_LEFT) & 0b1) pixels[p.x + width * p.y] = 0xffff0000;

		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(10);
	}

	window_destroy(window);

}