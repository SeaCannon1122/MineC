#include "testing.h"

#include <stdio.h>

#include "general/platformlib/platform.h"
#include "client/gui/menu.h"

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

	int menu_scale = 8;

	struct menu_label label = {
		MENU_ITEM_LABEL,
		0,
		20,
		-20,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		ALIGNMENT_MIDDLE,
		200,
		20,
		1,
		1,
		{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0} }
	};

	pixel_char_convert_string_in(label.text, "Hello World!\nMy name is SeaCannon1122\nHow are you?", 0xff00ffff, 0xff3f3f3f, PIXEL_CHAR_SHADOW_MASK);

	struct menu_scene menu = { -1, -1, -1, -1, 0, 1, {&label} };

	struct pixel_font* font = _load_char_font("../../../resources/client/assets/fonts/debug.pixelfont");

	show_console_window();
	
	int width = 800;
	int height = 500;

	int window = window_create(100, 100, width, height, "NAME");

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

		if (get_key_state(KEY_MOUSE_RIGHT)) if(p.x != -1) label.max_width = p.x - label.x * menu_scale;

		menu_scene_frame(&menu, pixels, width, height, menu_scale, &font, p.x, p.y);

		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(5);
	}

	window_destroy(window);

}