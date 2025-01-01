#pragma once

#ifndef GUI_MENUS_H
#define GUI_MENUS_H

#include <stdint.h>

#include "gui.h"

enum menus {
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
	MENU_JOIN_SERVER,
	MENU_INGAME_MAIN
};

struct gui_menus_state {

	uint32_t active_menu;
	
	struct {
		void* menu_handle;

		void* title_label_handle;

	} main;
};

struct game_client;

uint32_t gui_menus_create(struct game_client* game);

uint32_t gui_menus_destroy(struct game_client* game);

uint32_t gui_menus_simulation_frame(struct game_client* game);

uint32_t gui_menus_render(struct game_client* game);

#endif // !GUI_MENUS_H
