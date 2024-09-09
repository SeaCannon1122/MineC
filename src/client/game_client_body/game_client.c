#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>

#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"

#include "general/utils.h"
#include "general/keyvalue.h"
#include "general/resource_loader.h"
#include "general/resource_manager.h"

#include "client/gui/char_font.h"
#include "client/gui/menu.h"
#include "game_menus.h"

struct game_client* new_game_client(char* resource_path) {

	struct game_client* game = malloc(sizeof(struct game_client));
	if (game == NULL) return NULL;

	game->resource_manager = new_resource_manager(resource_path);

	struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;

	game->settings.render_distance = get_value_from_key(settings_map, "render_distance").i;
	game->settings.gui_scale = get_value_from_key(settings_map, "gui_scale").i;
	game->settings.resolution_scale = get_value_from_key(settings_map, "resolution_scale").i;
	game->settings.fov = get_value_from_key(settings_map, "fov").i;

	game->settings.resolution_scale = 2;

	init_game_menus(game);

	return game;
}

void run_game_client(struct game_client* game) {

	show_console_window();

	game->window = create_window(200, 100, 1100, 700, "client");

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	while (is_window_active(game->window) && !game->game_menus.main_menu.quit_game_button_state) {

		int width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		int height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		struct point2d_int mousepos = get_mouse_cursor_position(game->window);
		mousepos.x /= game->settings.resolution_scale;
		mousepos.y /= game->settings.resolution_scale;
		char click = get_key_state(KEY_MOUSE_LEFT);

		int render_gui_scale = (game->settings.gui_scale != 0 ? clamp_int(game->settings.gui_scale, 1, (width - 350) / 350 + 1) : (width - 350) / 350 + 1);

		switch (game->game_menus.active_menu) {

		case MAIN_MENU: {
  			menu_scene_frame(&game->game_menus.main_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);
			if (game->game_menus.main_menu.options_button_state) {
				game->game_menus.main_menu.options_button_state = false;
				game->game_menus.active_menu = OPTIONS_MENU;
				break;
			}
			else if (game->game_menus.main_menu.join_game_button_state) {
				game->game_menus.main_menu.join_game_button_state = false;
				game->game_menus.active_menu = JOIN_GAME_MENU;
				break;
			}
			break;
		}

		case OPTIONS_MENU: {

			menu_scene_frame(&game->game_menus.options_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);

			game->settings.render_distance = (float)RENDER_DISTANCE_MIN + (game->game_menus.options_menu.render_distance_slider_state * ((float)RENDER_DISTANCE_MAX - (float)RENDER_DISTANCE_MIN));
			game->settings.fov = (float)FOV_MIN + (game->game_menus.options_menu.fov_slider_state * ((float)FOV_MAX - (float)FOV_MIN));

			if (game->game_menus.options_menu.done_button_state || get_key_state(KEY_ESCAPE) == 0b11) {
				game->game_menus.options_menu.done_button_state = false;
				game->game_menus.active_menu = MAIN_MENU;
				break;
			}
			else if (game->game_menus.options_menu.gui_scale_button_state) {
				game->game_menus.options_menu.gui_scale_button_state = false;
				game->settings.gui_scale = (game->settings.gui_scale + 1) % ((width + 350) / 350);

				if (game->settings.gui_scale > 0) {
					game->game_menus.options_menu.gui_scale_text[11].value = digit_to_char(game->settings.gui_scale);
					game->game_menus.options_menu.gui_scale_text[12].value = '\x1f';
					game->game_menus.options_menu.gui_scale_text[13].value = '\x1f';
					game->game_menus.options_menu.gui_scale_text[14].value = '\x1f';
				}
				else {
					game->game_menus.options_menu.gui_scale_text[11].value = 'A';
					game->game_menus.options_menu.gui_scale_text[12].value = 'u';
					game->game_menus.options_menu.gui_scale_text[13].value = 't';
					game->game_menus.options_menu.gui_scale_text[14].value = 'o';
				}
			}

			game->game_menus.options_menu.render_distance_text[17].value = (game->settings.render_distance < 10 ? '\x1f' : digit_to_char(game->settings.render_distance / 10));
			game->game_menus.options_menu.render_distance_text[18].value = digit_to_char(game->settings.render_distance % 10);

			game->game_menus.options_menu.fov_text[5].value = (game->settings.fov < 100 ? '\x1f' : digit_to_char(game->settings.fov / 100));
			game->game_menus.options_menu.fov_text[6].value = (game->settings.fov < 10 ? '\x1f' : digit_to_char((game->settings.fov / 10) % 10));
			game->game_menus.options_menu.fov_text[7].value = digit_to_char(game->settings.fov % 10);

			break;
		}

		case JOIN_GAME_MENU: {

			menu_scene_frame(&game->game_menus.join_game_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);
			if (game->game_menus.join_game_menu.back_button_state || get_key_state(KEY_ESCAPE) == 0b11) {
				game->game_menus.join_game_menu.back_button_state = false;

				if (game->game_menus.join_game_menu.ip_address_buffer_link != -1) {
					unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer_link);
					game->game_menus.join_game_menu.ip_address_buffer_link = -1;
				}
				if (game->game_menus.join_game_menu.port_buffer_link != -1) {
					unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer_link);
					game->game_menus.join_game_menu.port_buffer_link = -1;
				}
				game->game_menus.active_menu = MAIN_MENU;
				break;
			}
			if (string_length(game->game_menus.join_game_menu.ip_address_buffer) != 1 && string_length(game->game_menus.join_game_menu.port_buffer) != 1) game->game_menus.join_game_menu.join_game_button_enabled = true;
			else game->game_menus.join_game_menu.join_game_button_enabled = false;

			if (game->game_menus.join_game_menu.ip_address_box_selected && game->game_menus.join_game_menu.ip_address_buffer_link == -1) {
				game->game_menus.join_game_menu.ip_address_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer, sizeof(game->game_menus.join_game_menu.ip_address_buffer), string_length(game->game_menus.join_game_menu.ip_address_buffer) - 1);
			}
			else if (game->game_menus.join_game_menu.ip_address_box_selected == false && game->game_menus.join_game_menu.ip_address_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer_link);
				game->game_menus.join_game_menu.ip_address_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.port_box_selected && game->game_menus.join_game_menu.port_buffer_link == -1) {
				game->game_menus.join_game_menu.port_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer, sizeof(game->game_menus.join_game_menu.port_buffer), string_length(game->game_menus.join_game_menu.port_buffer) - 1);
			}
			else if (game->game_menus.join_game_menu.port_box_selected == false && game->game_menus.join_game_menu.port_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer_link);
				game->game_menus.join_game_menu.port_buffer_link = -1;
			}

		}

		}

		
		draw_to_window(game->window, pixels, width, height, game->settings.resolution_scale);

		free(pixels);

		sleep_for_ms(10);
	}

	close_window(game->window);

	return;
}

void delete_game_client(struct game_client* game) {
	destroy_resource_manager(game->resource_manager);
	delete_game_menus(game);
	free(game);
}