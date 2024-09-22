#include "game_client_control.h"

#include "general/platformlib/platform.h"

#include "game_client.h"

void game_client_control_thread_function(struct game_client* game) {

	while (game->running) {

		while (game->in_game_flag) {

			if (game->game_menus.active_menu == NO_MENU) {
				if (get_key_state(KEY_ESCAPE) == 0b11) game->game_menus.active_menu = INGAME_OPTIONS_MENU;
				else if (get_key_state('T') == 0b11) game->game_menus.active_menu = CHAT_MENU;

			}

			sleep_for_ms(20);
		}

		sleep_for_ms(500);

	}

}