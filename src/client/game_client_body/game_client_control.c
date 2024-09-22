#include "game_client_control.h"

#include "general/platformlib/platform.h"

#include "game_client.h"
#include "game_client_networker.h"
void game_client_control_thread_function(struct game_client* game) {

	while (game->running) {

		if (game->disconnect_flag == 2) {
			if (game->in_game_flag == true) game->game_menus.active_menu = CONNECTION_WAITING_MENU;
			game->disconnect_flag = 1;
			game->in_game_flag = false;
			if (game->networker.status != NETWORK_INACTIVE) game->networker.close_connection_flag = true;
		}

		if (game->networker.status == NETWORK_CONNECTED && game->in_game_flag == false && game->disconnect_flag == 0) {
			game->in_game_flag = true;
			game->game_menus.active_menu = NO_MENU;
		}

		else if (game->disconnect_flag == 1 && game->networker.status == NETWORK_INACTIVE) game->disconnect_flag = 0;

		if (game->in_game_flag) {

			if (game->game_menus.active_menu == NO_MENU) {
				if (get_key_state(KEY_ESCAPE) == 0b11 && is_window_selected(game->window)) game->game_menus.active_menu = INGAME_OPTIONS_MENU;
				else if (get_key_state('T') == 0b11 && is_window_selected(game->window)) game->game_menus.active_menu = CHAT_MENU;

			}

		}

		sleep_for_ms(20);

	}

}