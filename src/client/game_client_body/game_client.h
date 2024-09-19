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
		struct {
			int year;
			int month;
			int day;
			int hour;
			int minute;
			int second;
			long long unix_time;
		} start_time;
		char start_time_str[20];
	} session;
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
	struct {
		int max_render_distance;
		int max_message_length;
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