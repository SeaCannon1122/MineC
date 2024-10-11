#include "testing.h"

#include <stdio.h>

#include "general/platformlib/platform.h"
#include "client/gui/menu.h"

int scale = 8;

void char_c(int window, int character) {
	 
	scale = character - 'a' + 1;

}

void* _load_file(char* filename, int* size) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

	fclose(file);

	*size = fileSize;

	return buffer;
}


struct pixel_font* _load_char_font(char* src) {

	int size;
	struct pixel_font* font = _load_file(src, &size);
	//if (size != sizeof(struct pixel_font)) { free(font); return NULL; }

	return font;
}


int testing_main() {

	struct menu_label label = {
		MENU_ITEM_LABEL,
		0,
		0,
		0,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		100,
		20,
		1,
		3,
		{{0xff00ffff, 0xff3f3f3f, 'A', PIXEL_CHAR_BACKGROUND_MASK}, {0xff00ffff, 0xff3f3f3f, 'B', 0}, {0xff00ffff, 0xff3f3f3f, 'C', 0}, {0xff00ffff, 0xff3f3f3f, 'D', 0}, {0xff00ffff, 0xff3f3f3f, 'E', 0}, {0xff00ffff, 0xff3f3f3f, '\n', 0},{0xff00ffff, 0xff3f3f3f, 'G', 0}, {0xff00ffff, 0xff3f3f3f, 'H', 0}, {0xff00ffff, 0xff3f3f3f, 'I', 0}, {0xff00ffff, 0xff3f3f3f, 'J', 0}, {0xff00ffff, 0xff3f3f3f, 'K', 0}, {0xff00ffff, 0xff3f3f3f, 'L', 0}, {0xff00ffff, 0xff3f3f3f, 'M', 0}, {0xff00ffff, 0xff3f3f3f, 'N', 0}, {0xff00ffff, 0xff3f3f3f, 'O', 0}, {0xff00ffff, 0xff3f3f3f, 'P', 0}, {0xff00ffff, 0xff3f3f3f, 'Q', 0}, {0xff00ffff, 0xff3f3f3f, 'R', 0}, {0xff00ffff, 0xff3f3f3f, 'S', 0}, {0xff00ffff, 0xff3f3f3f, 'T', 0}, {0xff00ffff, 0xff3f3f3f, 'U', 0}, {0xff00ffff, 0xff3f3f3f, 'V', 0}, {0xff00ffff, 0xff3f3f3f, 'W', 0}, {0xff00ffff, 0xff3f3f3f, 'X', 0}, {0xff00ffff, 0xff3f3f3f, 'Y', 0}, {0xff00ffff, 0xff3f3f3f, 'Z', 0}, {0xff00ffff, 0xff3f3f3f, '\0', 0}}
	};

	struct gui_menu menu = { -1, -1, -1, -1, 0, 1, {&label} };

	struct pixel_font* font = _load_char_font("../../../resources/client/assets/fonts/debug.pixelfont");

	show_console_window();
	
	int width = 800;
	int height = 500;

	int window = window_create(100, 100, width, height, "NAME");

	int cid = window_add_char_callback(window, char_c);

	width = window_get_width(window);
	height = window_get_width(window);

	unsigned int* pixels = malloc(sizeof(unsigned int) * height * width);

	while (window_is_active(window)) {

		int new_width = window_get_width(window);
		int new_height = window_get_height(window);

		if (width != new_width || height != new_height) {
			height = new_height;
			width = new_width;
			free(pixels);
			pixels = malloc(sizeof(unsigned int) * height * width);
		}

		for (int i = 0; i < height * width; i++) pixels[i] = 0x12345;


		struct point2d_int p = window_get_mouse_cursor_position(window);

		if (get_key_state(KEY_MOUSE_LEFT)) label.max_width = - 2 * p.x + width;

		menu_frame(&menu, pixels, width, height, 2, &font, get_key_state(KEY_MOUSE_LEFT), p.x, p.y);

		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(5);
	}

	window_destroy(window);

}