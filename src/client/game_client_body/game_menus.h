#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "client/game_client_body/resources.h"
#include "game/game_constants.h"

#include "general/key_value.h"

#define main_menu_this_is_not_minecraft_label_text "This is not Minecraft!!!"
#define main_menu_join_game_label_text "Join Game"
#define main_menu_options_label_text "Options..."
#define main_menu_quit_game_label_text "Quit Game"

#define options_menu_options_label_text "Options"
#define options_menu_fov_label_text "FOV:    "
#define options_menu_render_distance_label_text "Render Distance:    Chunks"
#define options_menu_gui_scale_label_text "GUI Scale:     "
#define options_menu_done_label_text "Done"

enum blocks_menus {
	NO_MENU,
	MAIN_MENU,
	OPTIONS_MENU,
	JOIN_GAME_MENU,
	CONNECTION_WAITING_MENU,
	CHAT_MENU,
	INGAME_OPTIONS_MENU,
};

struct game_menu_label {
	int menu_item_type;
	int z;
	int x;
	int y;
	int alignment_x;
	int alignment_y;
	int text_alignment_x;
	int text_alignment_y;
	int max_width;
	int max_rows;
	int selectable;
	int hoverable;
	int text_size;
	struct pixel_char text[64];
};

struct game_menus {
	int a;
};

struct game_client;

void init_game_menus(struct game_client* game);

void switch_game_menu(struct game_client* game, int menu);

void game_menus_frame(struct game_client* game);

void exit_game_menus(struct game_client* game);
