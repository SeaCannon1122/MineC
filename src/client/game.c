
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "general/utils.h"
#include "general/resource_loader.h"
#include "general/resource_manager.h"
#include "client/gui/char_font.h"
#include "gui/menu.h"
#include "general/networking/client.h"
#include "client/game_menus.h"

struct game* new_game() {
	return NULL;
}

void run_game(struct game* game, char* resource_path) {

	platform_init();

	show_console_window();

	struct window_state* window = create_window(200, 100, 1100, 700, "client");

	resource_manager_init();
	
	struct menu_scene* menus = malloc(sizeof(struct menu_scene) * 3);

	struct main_menu_flags main_menu_flags = { false, true, false , true, false, true };
	struct options_menu_flags options_main_menu_flags = { 0, NULL, 0, NULL, false, true, false, true, false};

	int active_menu = MAIN_MENU;

	init_main_menu(&menus[0], &main_menu_flags);
	init_options_menu(&menus[1], &options_main_menu_flags);

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	while (is_window_active(window) && !main_menu_flags.quit_game) {
		
		int width = window->window_width;
		int height = window->window_height;

		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		struct point2d_int mousepos = get_mouse_cursor_position(window);

		bool click = get_key_state(KEY_MOUSE_LEFT) == 0b11;

		switch (active_menu) {

		case MAIN_MENU: {
			menu_scene_frame(&menus[0], 2, pixels, width, height, mousepos.x, mousepos.y, click);
			if (main_menu_flags.options) {
				main_menu_flags.options = false;
				active_menu = OPTIONS_MENU;
			}
			break;
		}

		case OPTIONS_MENU: {
			menu_scene_frame(&menus[1], 2, pixels, width, height, mousepos.x, mousepos.y, click);
			if (options_main_menu_flags.done) {
				options_main_menu_flags.done = false;
				active_menu = MAIN_MENU;
			}
			//else if (options_main_menu_flags);
			break;
		}

		case JOIN_GAME_MENU: {

		}

		default:
			break;
		}

		
		draw_to_window(window, pixels, width, height);

		free(pixels);

		sleep_for_ms(20);
	}

	resource_manager_exit();

	close_window(window);

	//platform_exit();

	return;
}