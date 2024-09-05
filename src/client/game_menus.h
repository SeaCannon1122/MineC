#pragma once

#include <stdbool.h>

#include "gui/menu.h"

enum blocks_menus {
	MAIN_MENU = 0,
	OPTIONS_MENU = 1,
	JOIN_GAME_MENU = 3,
};

struct main_menu_flags {
	bool join_game;
	bool join_game_enable;

	bool options;
	bool options_enable;

	bool quit_game;
	bool quit_game_enable;
};

struct options_menu_flags {
	float fov;
	struct gui_character* fov_text;

	float render_distance;
	struct gui_character* render_distance_text;

	bool done;
	bool done_enable;

	bool gui_scale;
	bool gui_scale_enable;
	struct gui_character* gui_scale_text;
};

void init_main_menu(struct menu_scene* menu, struct main_menu_flags* flags);

void init_options_menu(struct menu_scene* menu, struct options_menu_flags* flags);