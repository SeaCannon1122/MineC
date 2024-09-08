
#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>

#include "client/platform.h"
#include "general/utils.h"
#include "general/keyvalue.h"
#include "general/resource_loader.h"
#include "general/resource_manager.h"
#include "client/gui/char_font.h"
#include "client/gui/menu.h"
#include "general/networking/client.h"
#include "game_menus.h"

struct game_client* new_game_client(char* resource_path) {

	struct game_client* game = malloc(sizeof(struct game_client));
	game->resource_manager = new_resource_manager(resource_path);

	struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;

	game->settings.render_distance = get_value_from_key(settings_map, "render_distance").i;
	game->settings.gui_scale = get_value_from_key(settings_map, "gui_scale").i;
	game->settings.resolution_scale = get_value_from_key(settings_map, "resolution_scale").i;
	game->settings.fov = get_value_from_key(settings_map, "fov").i;

	init_game_menus(game);

	return game;
}

void run_game_client(struct game_client* game) {

	show_console_window();

	int window = create_window(200, 100, 1100, 700, "client");

	int active_menu = MAIN_MENU;

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	while (is_window_active(window) && !game->game_menus.main_menu.quit_game_button_state) {

		int width = get_window_width(window);
		int height = get_window_height(window);
		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		struct point2d_int mousepos = get_mouse_cursor_position(window);
		char click = get_key_state(KEY_MOUSE_LEFT);

		switch (active_menu) {

		case MAIN_MENU: {
  			menu_scene_frame(&game->game_menus.main_menu.menu, game->settings.gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);
			if (game->game_menus.main_menu.options_button_state) {
				game->game_menus.main_menu.options_button_state = false;
				active_menu = OPTIONS_MENU;
			}
			else if (game->game_menus.main_menu.join_game_button_state) {
				game->game_menus.main_menu.join_game_button_state = false;
				active_menu = JOIN_GAME_MENU;
			}
			break;
		}

		case OPTIONS_MENU: {

			game->game_menus.options_menu.render_distance_text[17].value = digit_to_char(game->settings.render_distance / 10);
			game->game_menus.options_menu.render_distance_text[18].value = digit_to_char(game->settings.render_distance % 10);

			menu_scene_frame(&game->game_menus.options_menu.menu, game->settings.gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);

			game->settings.render_distance = (float)RENDER_DISTANCE_MIN + (game->game_menus.options_menu.render_distance_slider_state * ((float)RENDER_DISTANCE_MAX - (float)RENDER_DISTANCE_MIN));

			if (game->game_menus.options_menu.done_button_state) {
				game->game_menus.options_menu.done_button_state = false;
				active_menu = MAIN_MENU;
			}
			else if (game->game_menus.options_menu.gui_scale_button_state) {
				game->game_menus.options_menu.gui_scale_button_state = false;
				game->settings.gui_scale = (game->settings.gui_scale) % 4 + 1;
				game->game_menus.options_menu.gui_scale_text[11].value = digit_to_char(game->settings.gui_scale);
			}

			//else if (options_main_menu_flags);
			break;
		}

		case JOIN_GAME_MENU: {
			menu_scene_frame(&game->game_menus.join_game_menu.menu, game->settings.gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);
			if (game->game_menus.join_game_menu.back_button_state) {
				game->game_menus.join_game_menu.back_button_state = false;
				active_menu = MAIN_MENU;
			}
		}

		default:
			break;
		}

		
		draw_to_window(window, pixels, width, height);

		free(pixels);

		sleep_for_ms(10);
	}



	close_window(window);

	return;
}