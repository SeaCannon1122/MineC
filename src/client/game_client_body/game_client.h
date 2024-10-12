#pragma once

#include "general/keyvalue.h"
#include "game_menus.h"
#include "game/game_constants.h"
#include "game_client_networker.h"
#include "game_client_renderer.h"
#include "game/game_constants.h"
#include "game/chat.h"
#include "game/chunk.h"

enum game_request {
	NULL_FLAG,
	SHOULD_CONNECT,
};

struct chat_stream_element {
	long long time;
	struct game_chat_char message[MAX_SERVER_MESSAGE_LENGTH + 1];
	char is_chat_line_break[MAX_SERVER_MESSAGE_LENGTH + 1];
};

struct chunk_table_entry {
	struct {
		int x;
		int y;
		int z;
	} coordinates;
	struct game_raw_chunk *chunk;
	char state;
};

struct game_client {
	void* resource_map[200];
	char resource_folder_path[1024];
	FILE* debug_log_file;
	FILE* chat_log_file;
	struct {
		int next_index;
		struct chat_stream_element* stream;
	} chat_stream;
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
	struct game_renderer renderer;
	bool rendering;
	struct {
		struct chunk_table_entry* chunk_table;
		int chunk_table_length;

		struct {
			struct global_position position;
			struct direction direction;

		} player;

	} game_state;

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
		int chat_width;
		int max_chat_lines_display;
		int chat_display_duration;
		int chat_stream_length;
		int chat_indentation_left;
		int chat_indentation_right;
		int chat_line_radius;
		int max_chunk_requests;
	} constants;
	struct game_menus game_menus;
};

void log_chat_message(struct game_client* game);

int new_game_client(struct game_client* game, char* resource_path);

void run_game_client(struct game_client* game);

void delete_game_client(struct game_client* game);