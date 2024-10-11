#include "testing.h"

#include <stdio.h>

#include "general/platformlib/platform.h"
#include "client/gui/pixel_char.h"

int scale = 15;

void char_c(int window, int character) {
	int str[2] = { character,  0};
	 
	scale = character - 'a' + 1;

	printf(str);
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

	struct pixel_font* font = _load_char_font("../../../resources/client/assets/fonts/debug.pixelfont");

	show_console_window();
	
	pixel_char_convert_string(word, "[Hello} World!", 0xff00ffff, PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK | PIXEL_CHAR_SHADOW_MASK);

	struct pixel_char word1[] = {
		{0xffffffff, 'H', 0},
		{0xffffffff, 'e', PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'l', PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'l', PIXEL_CHAR_CURSIVE_MASK},
		{0xffffffff, 'o', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, ',', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, ' ', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'W', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'h', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'a', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 't', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, '\'', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 's', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, ' ', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'u', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, 'p', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, '!', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},
		{0xffffffff, '\0', PIXEL_CHAR_CURSIVE_MASK | PIXEL_CHAR_UNDERLINE_MASK},

	};

	int width = 800;
	int height = 500;

	int window = window_create(100, 100, width, height, "NAME");

	int cid = window_add_char_callback(window, char_c);

	width = window_get_width(window);
	height = window_get_width(window);

	unsigned int* pixels = malloc(sizeof(unsigned int) * height * width);

	while (get_key_state(KEY_ESCAPE) == 0 && window_is_active(window)) {

		int new_width = window_get_width(window);
		int new_height = window_get_height(window);

		if (width != new_width || height != new_height) {
			height = new_height;
			width = new_width;
			free(pixels);
			pixels = malloc(sizeof(unsigned int) * height * width);
		}
		for (int i = 0; i < height * width; i++) pixels[i] = 0x12345;

		pixel_char_print_string(word, scale, 100, 100, pixels, width, height, &font);

		struct point2d_int p = window_get_mouse_cursor_position(window);

		if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height && get_key_state(KEY_MOUSE_LEFT) & 0b1) pixels[p.x + width * p.y] = 0xffff0000;

		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(10);
	}

	window_destroy(window);

}