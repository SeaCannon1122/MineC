#pragma once

#include <stdbool.h>

#include "gui/menu.h"

struct main_menu_flags {
	bool join_game;
	bool join_game_enable;

	bool options;
	bool options_enable;

	bool quit_game;
	bool quit_game_enable;
};

void init_main_menu(struct menu_scene* menu, struct main_menu_flags* flags);