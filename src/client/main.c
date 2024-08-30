#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "general/utils.h"
#include "general/parallel_computing.h"

#include "gui/menu.h"

#include "client/gui/char_font.h"

void Entry() {
	//show_console_window();

	struct window_state* window = create_window(200, 100, 700,500, "client");

	printf("Hello from client!\n");

	unsigned int pattern[] = { 
		0xff000000
	};

	struct argb_image checker = { 1, 1, pattern };

	

	

	struct char_font* default_font = load_char_font("resources/gui/default.pixelfont");

	int font_width = 4;


	struct menu_scene ms = { {0}, 0, NULL, 0, };

	char str1[] = "This is a game!";
	char str2[] = "This is going to be a minecraft clone one day.";

	struct gui_character* str1_gui = convert_string_to_gui_string(default_font, str1);
	struct gui_character* str2_gui = convert_string_to_gui_string(default_font, str2);

	add_menu_label(&ms, 0, 0, -30, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, str1_gui, ALIGNMENT_MIDDLE);
	add_menu_label(&ms, 0, 0, -20, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, str2_gui, ALIGNMENT_MIDDLE);

	while (!get_key_state(KEY_ESCAPE) && is_window_active(window)) {
		int width = window->window_width;
		int height = window->window_height;

		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				pixels[i + width * j] = 0xffff0000;
			}
		}

		render_menu_scene(&ms, 4, pixels, width, height);

		draw_to_window(window, pixels, width, height);

		free(pixels);

		sleep_for_ms(10);
	}

	

	printf("done!\n");

	while (!get_key_state(KEY_ESCAPE) && is_window_active(window)) sleep_for_ms(10);

	close_window(window);

	return;
}