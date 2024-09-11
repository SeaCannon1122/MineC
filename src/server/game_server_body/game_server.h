#pragma once

#include "general/keyvalue.h"

struct game_server {
	struct key_value_map* resource_manager;
};

void new_game_server(struct game_server* game, char* resource_path);

void run_game_server(struct game_server* game);

void delete_game_server(struct game_server* game);