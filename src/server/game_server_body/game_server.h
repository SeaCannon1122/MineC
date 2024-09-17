#pragma once

#include <stdbool.h>

#include "general/keyvalue.h"
#include "game/game_constants.h"

#define MAX_CLIENTS 4

struct game_server {
	struct key_value_map* resource_manager;
	struct key_value_map* username_password_map;
	void* server_handle;
	struct {
		void* client_handle;
		char username[MAX_USERNAME_LENGTH + 1];
		char ip_address[22 + 1];
		unsigned short port;
	} clients[MAX_CLIENTS];
	bool running;
};

void new_game_server(struct game_server* game, char* resource_path);

void run_game_server(struct game_server* game);

void delete_game_server(struct game_server* game);