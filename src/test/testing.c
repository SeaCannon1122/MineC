#include "testing.h"

#include <stdio.h>

#include "general/platformlib/platform.h"
#include "client/gui/menu.h"
#include "key_value.h"


int testing_main() {
	show_console_window();
	
	void* settings = key_value_load_yaml("../../../resources/client/settings.yaml");

	float floating = 0;
	int ret_status = key_value_get_float(settings, "val3", 100.f, &floating);
	printf("%f\n", floating);
	char buffer[64] = "Helloqw wefw2f";
	ret_status = key_value_get_string(settings, "val4", 100, buffer, 64);
	printf("%s\n", buffer);

	free(settings);

	void* lib = dynamic_library_load("test_lib");

	int (*my_add)(int, int) = dynamic_library_get_function(lib, "my_add");

	//dynamic_library_unload(lib);

	int result = my_add(3, 187);
	printf("%d\n", result);
	fflush(stdout);

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



		window_draw(window, pixels, width, height, 1);

		sleep_for_ms(5);
	}

	window_destroy(window);

}