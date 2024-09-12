#include "debug.h"

void debug_init(struct game_client* game) {
	game->game_menus.join_game_menu.ip_address_buffer[0] = '1';
	game->game_menus.join_game_menu.ip_address_buffer[1] = '2';
	game->game_menus.join_game_menu.ip_address_buffer[2] = '7';
	game->game_menus.join_game_menu.ip_address_buffer[3] = '.';
	game->game_menus.join_game_menu.ip_address_buffer[4] = '0';
	game->game_menus.join_game_menu.ip_address_buffer[5] = '.';
	game->game_menus.join_game_menu.ip_address_buffer[6] = '0';
	game->game_menus.join_game_menu.ip_address_buffer[7] = '.';
	game->game_menus.join_game_menu.ip_address_buffer[8] = '1';

	game->game_menus.join_game_menu.port_buffer[0] = '8';
	game->game_menus.join_game_menu.port_buffer[1] = '0';
	game->game_menus.join_game_menu.port_buffer[2] = '8';
	game->game_menus.join_game_menu.port_buffer[3] = '0';

	game->game_menus.join_game_menu.username_buffer[0] = '1';

	game->game_menus.join_game_menu.password_buffer[0] = '1';
}