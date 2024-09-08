#pragma once

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "general/keyvalue.h"
#include "game_menus.h"

#define FOV_MIN 30
#define FOV_MAX 120

#define RENDER_DISTANCE_MIN 2
#define RENDER_DISTANCE_MAX 32

struct game_client {
	struct key_value_map* resource_manager;
	struct {
		int render_distance;
		int fov;
		int gui_scale;
		int resolution_scale;
	} settings;
	struct game_menus game_menus;
};

struct game_client* new_game_client(char* resource_path);

void run_game_client(struct game_client* game);

#endif // !GAME_CLIENT_H