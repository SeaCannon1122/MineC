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
#include "game_client_networker.h"

void new_game_client(struct game_client* game, char* resource_path) {

	game->resource_manager = new_resource_manager(resource_path);

	struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;

	game->settings.render_distance = get_value_from_key(settings_map, "render_distance").i;
	game->settings.gui_scale = get_value_from_key(settings_map, "gui_scale").i;
	game->settings.resolution_scale = get_value_from_key(settings_map, "resolution_scale").i;
	game->settings.fov = get_value_from_key(settings_map, "fov").i;

	init_game_menus(game);
	init_networker(game);

	return game;
}

void run_game_client(struct game_client* game) {
	show_console_window();

	game->window = create_window(200, 100, 1100, 700, "client");

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	int render_width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	int render_height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;

	while (is_window_active(game->window) && !game->game_menus.main_menu.quit_game_button_state) {

		int width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		int height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		
		game_menus_frame(game, pixels, width, height);

		
		draw_to_window(game->window, pixels, width, height, game->settings.resolution_scale);

		free(pixels);

		sleep_for_ms(10);
	}

	close_window(game->window);

	return;
}

void delete_game_client(struct game_client* game) {
	destroy_resource_manager(game->resource_manager);
}