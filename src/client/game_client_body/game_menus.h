#pragma once

#ifndef GAME_MENUS_H
#define GAME_MENUS_H

#include <stdbool.h>

#include "client/gui/menu.h"
#include "client/gui/char_font.h"

#include "general/keyvalue.h"

enum blocks_menus {
	MAIN_MENU = 0,
	OPTIONS_MENU = 1,
	JOIN_GAME_MENU = 3,
	CONNECTION_WAITING_MENU = 4,
};

struct game_menus {
	struct main_menu {
		struct gui_character this_is_not_minecraft_text[25];

		struct gui_character join_game_text[10];
		bool join_game_button_state;
		bool join_game_button_enabled;

		struct gui_character options_text[11];
		bool options_button_state;
		bool options_button_enabled;

		struct gui_character quit_game_text[10];
		bool quit_game_button_state;
		bool quit_game_button_enabled;

		struct menu_scene menu;
	} main_menu;

	struct options_menu {
		struct gui_character options_text[8];

		float fov_slider_state;
		struct gui_character fov_text[9];

		float render_distance_slider_state;
		struct gui_character render_distance_text[27];

		bool gui_scale_button_state;
		bool gui_scale_button_enabled;
		struct gui_character gui_scale_text[16];

		bool done_button_state;
		bool done_button_enabled;
		struct gui_character done_text[5];

		struct menu_scene menu;
	} options_menu;

	struct join_game_menu {
		struct gui_character join_a_multiplayer_game_text[24];

		struct gui_character ip_address_text[12];

		char ip_address_buffer[16 + 1];
		bool ip_address_box_selected;
		int ip_address_buffer_link;

		struct gui_character port_text[6];

		char port_buffer[5 + 1];
		bool port_box_selected;
		int port_buffer_link;

		struct gui_character username_text[10];

		char username_buffer[16 + 1];
		bool username_box_selected;
		int username_buffer_link;

		struct gui_character password_text[10];

		char password_buffer[16 + 1];
		bool password_box_selected;
		int password_buffer_link;

		struct gui_character join_game_text[10];
		bool join_game_button_state;
		bool join_game_button_enabled;

		bool back_button_state;
		bool back_button_enabled;
		struct gui_character back_text[5];

		struct menu_scene menu;
	} join_game_menu;

	struct {

		struct gui_character networking_message[64];

		bool back_button_state;
		bool back_button_enabled;
		struct gui_character back_text[5];

		struct menu_scene menu;
	} connection_waiting_menu;

	int active_menu;
};

struct game_client;

void init_game_menus(struct game_client* game);

void game_menus_frame(struct game_client* game, unsigned int* pixels, int width, int height);

#endif // !GAME_MENUS_H