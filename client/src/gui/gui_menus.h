#pragma once

#ifndef GUI_MENUS_H
#define GUI_MENUS_H

#include <stdint.h>

#include "gui.h"

enum menus {
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
	MENU_JOIN_GAME,
	MENU_SERVER_INTERMEDIATE,
	MENU_INGAME_MAIN
};

struct gui_menus_state {

	uint32_t active_menu;
	
	struct {
		void* menu_handle;

		void* title_label;

		void* join_game_button;
		void* join_game_label;

		void* options_button;
		void* options_label;

		void* quit_game_button;
		void* quit_game_label;

	} main;

	struct {
		void* menu_handle;

		void* join_a_server_label;

		void* ip_label;
		void* ip_textfield;
		uint32_t* ip_textfield_ptr;
		void* invalid_syntax_label;

		void* username_label;
		void* username_textfield;
		uint32_t* username_textfield_ptr;

		void* password_label;
		void* password_textfield;
		uint32_t* password_textfield_ptr;

		void* join_label;
		void* join_button;
		
		void* back_label;
		void* back_button;

	} join_game;

	struct {
		void* menu_handle;

		void* status_label;
		uint8_t status;
		uint8_t inactive_reason;

		void* message_label;

		void* back_label;
		void* back_button;

	} server_intermediate;
};

struct minec_client;

uint32_t gui_menus_create(struct minec_client* game);

uint32_t gui_menus_destroy(struct minec_client* game);

uint32_t gui_menus_simulation_frame(struct minec_client* game);

uint32_t gui_menus_render(struct minec_client* game);

#endif // !GUI_MENUS_H
