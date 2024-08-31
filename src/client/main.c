#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "general/utils.h"
#include "general/resource_manager.h"

#include "gui/menu.h"

#include "client/gui/char_font.h"

void Entry() {
	//show_console_window();

	struct window_state* window = create_window(200, 100, 1100,700, "client");

	struct argb_image* dirt = load_png("resources/textures/dirt.png");

	struct char_font* default_font = load_char_font("resources/gui/default.pixelfont");

	struct menu_scene ms = { {0}, 0, dirt, 10 };

	char str1[] = "This is a game!";
	char str2[] = "This is going to be a minecraft clone one day.";
	char str3[] = "this is the left site";
	char str4[] = "this is the right site";

	struct gui_character* str1_gui = convert_string_to_gui_string(default_font, str1);
	struct gui_character* str2_gui = convert_string_to_gui_string(default_font, str2);
	struct gui_character* str3_gui = convert_string_to_gui_string(default_font, str3);
	struct gui_character* str4_gui = convert_string_to_gui_string(default_font, str4);

	add_menu_label(&ms, 0, 0, 20, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, str1_gui, ALIGNMENT_MIDDLE);
	add_menu_label(&ms, 1, 0, -20, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, str2_gui, ALIGNMENT_MIDDLE);
	add_menu_label(&ms, 2, 10, 0, ALIGNMENT_LEFT, ALIGNMENT_MIDDLE, str3_gui, ALIGNMENT_LEFT);
	add_menu_label(&ms, 3, -10, 0, ALIGNMENT_RIGHT, ALIGNMENT_MIDDLE, str4_gui, ALIGNMENT_RIGHT);

	while (!get_key_state(KEY_ESCAPE) && is_window_active(window)) {
		int width = window->window_width;
		int height = window->window_height;

		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

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