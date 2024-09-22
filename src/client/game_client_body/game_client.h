#pragma once

#include "general/keyvalue.h"
#include "game_menus.h"
#include "game/game_constants.h"
#include "game_client_networker.h"

enum game_request {
	NULL_FLAG,
	SHOULD_CONNECT,
	SHOULD_ABORT_CONNECTING,
};

struct game_client {
	char resource_folder_path[1024];
	FILE* debug_log_file;
	FILE* chat_log_file;
	bool running;
	int window;
	struct key_value_map* resource_manager;
	struct game_networker networker;
	bool in_game_flag;
	int disconnect_flag;
	struct {
		int width;
		int height;
		unsigned int* pixels;
	} render_state;
	struct {
		int render_distance;
		int fov;
		int gui_scale;
		int resolution_scale;
	} settings;
	struct {
		int max_render_distance;
	} server_settings;
	struct {
		int render_distance_min;
		int render_distance_max;
		int fov_min;
		int fov_max;
		int client_connection_timeout;
		int packet_awaiting_timeout;
	} constants;
	struct game_menus game_menus;
};

int new_game_client(struct game_client* game, char* resource_path);

void run_game_client(struct game_client* game);

void delete_game_client(struct game_client* game);