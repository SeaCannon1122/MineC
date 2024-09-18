#pragma once

#include <stdbool.h>

#include "general/keyvalue.h"
#include "game/game_constants.h"

struct client_on_server {
	void* client_handle;
	char username[MAX_USERNAME_LENGTH + 1];
	char ip_address[22 + 1];
	unsigned short port;
};

struct game_server {
	struct key_value_map* resource_manager;
	struct key_value_map* username_password_map;
	void* server_handle;
	struct client_on_server* clients;
	int clients_connected_count;
	int clients_length;
	bool running;
	struct {
		int max_clients;
		int max_render_distance;
		int max_message_length;
	} settings;
};

void new_game_server(struct game_server* game, char* resource_path);

void run_game_server(struct game_server* game);

void delete_game_server(struct game_server* game);