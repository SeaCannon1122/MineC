#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "src/general/utils.h"
#include "src/general/parallel_computing.h"

#include "gui/menu.h"

#include "gui/char_font.h"

void Entry() {
	show_console_window();

	struct window_state* window = create_window(200, 100, 700, 500, "client");

	printf("Hello from client!\n");

	int width = window->window_width;
	int height = window->window_height;

	unsigned int pattern[] = { 
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xffffffff, 0xff000000, 0xff000000, 0xffffffff, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xffffffff, 0xff000000, 0xff000000, 0xffffffff, 0xff000000,
		0xff000000, 0xff000000, 0xffffffff, 0xffffffff, 0xff000000, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	};

	struct argb_image checker = { 6, 6, pattern };

	struct menu_scene main_menu = { {0}, {0}, &checker, 3.2f };

	unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

	render_menu_scene(&main_menu, pixels, width, height);



	draw_to_window(window, pixels, width, height);

	printf("done!\n");

	while (!get_key_state(KEY_ESCAPE)) sleep_for_ms(10);

	close_window(window);

	return;
}