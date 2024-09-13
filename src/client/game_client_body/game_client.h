#pragma once

#include "general/keyvalue.h"
#include "game_menus.h"
#include "game/game_constants.h"

enum game_request {
	NULL_FLAG,
	SHOULD_CONNECT,
	SHOULD_ABORT_CONNECTING,
};

struct game_client {
	bool running;
	int window;
	struct key_value_map* resource_manager;
	struct {
		void* network_handle;
		int status;
		int message;
		int request;
		int port;
		char ip[15 + 1];
		char username[MAX_USERNAME_LENGTH + 1];
		char password[MAX_PASSWORD_LENGTH + 1];
		bool close_connection_flag;
	} networker;
	int game_flag;
	struct {
		int render_distance;
		int fov;
		int gui_scale;
		int resolution_scale;
	} settings;
	struct game_menus game_menus;
};

void new_game_client(struct game_client* game, char* resource_path);

void run_game_client(struct game_client* game);

void delete_game_client(struct game_client* game);