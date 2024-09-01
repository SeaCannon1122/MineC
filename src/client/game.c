#include "game.h"

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "general/utils.h"
#include "general/resource_manager.h"
#include "client/gui/char_font.h"
#include "gui/menu.h"


struct game* new_game() {

}

void run_game(struct game* game, char* resource_path) {
	//show_console_window();

	struct window_state* window = create_window(200, 100, 1100, 700, "client");

	struct argb_image* background = load_argb_image_from_png("resources/gui/menu/background.png");

	struct argb_image* button_texture_enabled = load_argb_image_from_png("resources/gui/menu/button_texture_enabled.png");
	struct argb_image* button_texture_disabled = load_argb_image_from_png("resources/gui/menu/button_texture_disabled.png");

	struct char_font* default_font = load_char_font("resources/gui/default.pixelfont");

	struct menu_scene ms = { {0}, 0 };

	char str1[] = "This is a game!";
	char str2[] = "This is going to be a minecraft clone one day.";
	char str3[] = "this is the left site";
	char str4[] = "this is the right site";
	char str5[] = "Button!!!";

	struct gui_character* str1_gui = convert_string_to_gui_string(default_font, str1);
	struct gui_character* str2_gui = convert_string_to_gui_string(default_font, str2);
	struct gui_character* str3_gui = convert_string_to_gui_string(default_font, str3);
	struct gui_character* str4_gui = convert_string_to_gui_string(default_font, str4);
	struct gui_character* str5_gui = convert_string_to_gui_string(default_font, str5);

	add_menu_label(&ms, 0, 0, 20, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, str1_gui, ALIGNMENT_MIDDLE);
	add_menu_label(&ms, 1, 0, -20, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, str2_gui, ALIGNMENT_MIDDLE);
	add_menu_label(&ms, 2, 10, 0, ALIGNMENT_LEFT, ALIGNMENT_MIDDLE, str3_gui, ALIGNMENT_LEFT);
	add_menu_label(&ms, 3, -10, 0, ALIGNMENT_RIGHT, ALIGNMENT_MIDDLE, str4_gui, ALIGNMENT_RIGHT);
	add_menu_label(&ms, 5, 0, 0, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, str5_gui, ALIGNMENT_MIDDLE);
	add_menu_image(&ms, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 10);

	bool button_state;
	bool button_enabled = true;

	add_menu_button(&ms, 4, &button_state, -100, -10, 100, 10, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, button_texture_enabled, button_texture_disabled, &button_enabled);

	while (!get_key_state(KEY_ESCAPE) && is_window_active(window)) {
		int width = window->window_width;
		int height = window->window_height;

		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		struct point2d_int mousepos = get_mouse_cursor_position(window);

		render_menu_scene(&ms, 2, pixels, width, height, mousepos.x, mousepos.y);

		draw_to_window(window, pixels, width, height);

		free(pixels);

		sleep_for_ms(10);
	}



	printf("done!\n");

	while (!get_key_state(KEY_ESCAPE) && is_window_active(window)) sleep_for_ms(10);

	close_window(window);
	return;
}