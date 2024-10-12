#include "game_client_simulator.h"

#include "general/platformlib/platform.h"

void game_client_simulator_thread_function(struct game_client* game) {

	while (game->running) {

		if (game->disconnect_flag == 2) {
			if (game->in_game_flag == true) switch_game_menu(game, CONNECTION_WAITING_MENU);
			game->disconnect_flag = 1;
			game->in_game_flag = false;
			if (game->networker.status != NETWORK_INACTIVE) game->networker.close_connection_flag = true;
		}

		if (game->networker.status == NETWORK_CONNECTED && game->in_game_flag == false && game->disconnect_flag == 0) {
			game->in_game_flag = true;
			switch_game_menu(game, NO_MENU);
		}

		else if (game->disconnect_flag == 1 && game->networker.status == NETWORK_INACTIVE) game->disconnect_flag = 0;

		if (game->in_game_flag) {

			if (game->game_menus.active_menu == NO_MENU) {
				if (get_key_state(KEY_ESCAPE) == 0b11 && window_is_selected(game->window)) switch_game_menu(game, INGAME_OPTIONS_MENU);
				else if (get_key_state('T') == 0b11 && window_is_selected(game->window)) switch_game_menu(game, CHAT_MENU);

			}

		}

		sleep_for_ms(20);

	}

}