#pragma once

#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#define CLIENT_CHAT_QUEUE_LENGTH 8

struct client_on_server {
	void* client_handle;
	char username[MAX_USERNAME_LENGTH + 1];
	char ip_address[22 + 1];
	unsigned short port;
	int next_packet_type;
	bool logged_in_from_another_location;
	bool just_logged_in;
};

struct game_server {
	char resource_folder_path[1024];
	FILE* debug_log_file;
	FILE* chat_log_file;
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

uint32_t game_server_run(struct game_server* game, uint8_t resource_path);

#endif // !GAME_SERVER_H





