#pragma once

#include "general/keyvalue.h"

#define MAX_CLIENTS 16

struct game_server {
	struct key_value_map* resource_manager;
	struct key_value_map* username_password_map;
	void* client_handles[MAX_CLIENTS];
	int currently_connected;
};

void new_game_server(struct game_server* game, char* resource_path);

void run_game_server(struct game_server* game);

void delete_game_server(struct game_server* game);