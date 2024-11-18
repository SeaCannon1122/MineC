#include "testing.h"

#include <stdio.h>
#include <stdlib.h>
#include "pixel_char.h"
#include "key_value.h"
#include "general/platformlib/platform/platform.h"

uint8_t* read_text_file(uint8_t* filename) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	uint8_t* buffer_raw = (uint8_t*)malloc(fileSize);
	if (buffer_raw == NULL) {
		fclose(file);
		return NULL;
	}

	size_t bytesRead = fread(buffer_raw, sizeof(uint8_t), fileSize, file);

	int32_t carriage_return_count = 0;

	for (int32_t i = 0; i < fileSize; i++) if (buffer_raw[i] == '\r') carriage_return_count++;

	uint8_t* buffer = (uint8_t*)malloc(fileSize + 1 - carriage_return_count);

	int32_t i = 0;

	for (int32_t raw_i = 0; raw_i < fileSize; raw_i++) {
		if (buffer_raw[raw_i] != '\r') {
			buffer[i] = buffer_raw[raw_i];
			i++;
		}
	}

	buffer[fileSize - carriage_return_count] = '\0';
	free(buffer_raw);

	fclose(file);

	return buffer;
}


int32_t testing_main() {
	show_console_window();

	pixel_char_init();

	struct pixel_font* font = load_pixel_font("../../../resources/client/assets/fonts/debug.pixelfont");



	pixel_char_convert_string(text_string, "Hello World!\nThis is so cool", 0xff00ffff, 0xffffffff, 0);

	int32_t width = 700;
	int32_t height = 500;

	int32_t window = window_create(100, 100, width, height, "NAME");

	width = window_get_width(window);
	height = window_get_width(window);

	uint32_t* pixels = malloc(sizeof(uint32_t) * height * width);

	GLCall(glClearColor(0.92f, 0.81f, 0.53f, 1.0f));

	GLCall(glViewport(0, 0, width, height));

	while (window_is_active(window) && !get_key_state(KEY_ESCAPE)) {

		int32_t new_width = window_get_width(window);
		int32_t new_height = window_get_height(window);

		if (width != new_width || height != new_height) {
			height = new_height;
			width = new_width;
			free(pixels);
			pixels = malloc(sizeof(uint32_t) * height * width);

		}


		pixel_char_print_string_vk(text_string, 10, 1, width / 2, height / 2, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, 1000000, 0, pixels, width, height, &font);

		//pixel_uint8_t_print32_t_string(text_string, 5, 1, 300, 300, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, 1000000, 0, pixels, width, height, &font);


		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(5);
	}


	window_destroy(window);

}