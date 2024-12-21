#ifndef MENUS_H
#define MENUS_H

#include <stdint.h>

enum menus {
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
	MENU_JOIN_SERVER,
	MENU_INGAME_MAIN
};

struct menus_state {

	uint32_t menu;
	

};

struct game_client;

uint32_t menus_simulate(struct game_client* game);

#endif // !MENUS_H
