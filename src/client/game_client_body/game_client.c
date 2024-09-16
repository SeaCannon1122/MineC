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
#include "debug.h"

void new_game_client(struct game_client* game, char* resource_path) {

	game->resource_manager = new_resource_manager(resource_path);

	struct key_value_map* settings_map = get_value_from_key(game->resource_manager, "settings").ptr;

	game->settings.render_distance = get_value_from_key(settings_map, "render_distance").i;
	game->settings.gui_scale = get_value_from_key(settings_map, "gui_scale").i;
	game->settings.resolution_scale = get_value_from_key(settings_map, "resolution_scale").i;
	game->settings.fov = get_value_from_key(settings_map, "fov").i;

	game->running = false;

	init_game_menus(game);
	init_networker(game);
	debug_init(game);
}

void run_game_client(struct game_client* game) {
	show_console_window();

	game->running = true;

	game->window = create_window(200, 100, 1100, 700, "client");

	void* networking_thread = create_thread((void(*)(void*))networker_thread, game);
	//void* rendering_thread;

	while (get_key_state(KEY_MOUSE_LEFT) & 0b1) sleep_for_ms(10);

	int render_width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
	int render_height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;

	while (is_window_active(game->window) && !game->game_menus.main_menu.quit_game_button_state) {

		int width = (get_window_width(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		int height = (get_window_height(game->window) + game->settings.resolution_scale - 1) / game->settings.resolution_scale;
		unsigned int* pixels = malloc(width * height * sizeof(unsigned int));

		//render_ingame_frame(game, pixels, width, height);
		
		game_menus_frame(game, pixels, width, height);

		switch (game->game_flag)
		{

		case SHOULD_CONNECT: {
			if(game->networker.status == NETWORK_INACTIVE) {
				game->networker.port = string_to_int(game->game_menus.join_game_menu.port_buffer, sizeof(game->game_menus.join_game_menu.port_buffer) - 1);
				for (int i = 0; i < 16; i++) game->networker.ip[i] = game->game_menus.join_game_menu.ip_address_buffer[i];
				for (int i = 0; i < MAX_USERNAME_LENGTH; i++) game->networker.username[i] = game->game_menus.join_game_menu.username_buffer[i];
				for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) game->networker.password[i] = game->game_menus.join_game_menu.password_buffer[i];
				game->networker.request = CONNECT_TO_SERVER;
			}
			game->game_flag = NULL_FLAG;
		} break;

		case SHOULD_ABORT_CONNECTING: {
			if (game->networker.status == NETWORK_CONNECTED) game->networker.close_connection_flag = true;
			game->game_flag = NULL_FLAG;
		} break;


		default:
			break;
		}

		game->game_flag = NULL_FLAG;

		


		draw_to_window(game->window, pixels, width, height, game->settings.resolution_scale);

		free(pixels);

		sleep_for_ms(10);
	}

	close_window(game->window);
	game->networker.close_connection_flag = true;

	while (game->networker.network_handle != NULL) sleep_for_ms(1);

	game->running = false;

	//join_thread(rendering_thread);
	join_thread(networking_thread);


	return;
}



void delete_game_client(struct game_client* game) {
	destroy_resource_manager(game->resource_manager);
}