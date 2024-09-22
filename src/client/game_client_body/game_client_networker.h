#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "game/game_constants.h"

enum networking_status {
	NETWORK_INACTIVE,
	NETWORK_CONNECTING, 
	NETWORK_CONNECTED,

};

enum networker_request_type {
	NULL_REQUEST,
	CONNECT_TO_SERVER, 
	DISCONNECT_FROM_SERVER,
	SEND_GAME_PACKET,
};

struct game_networker {
	void* network_handle;
	int next_packet_type;
	int status;
	int request;
	int port;
	char ip[15 + 1];
	char username[MAX_USERNAME_LENGTH + 1];
	char password[MAX_PASSWORD_LENGTH + 1];
	bool close_connection_flag;
	int status_updates_missed;
	int send_packet_type;
	struct {
		void* data;
		size_t size;
	} send_data[3];
	bool should_send_chat_message;
	char send_chat_message[MAX_CHAT_MESSSAGE_LENGTH + 1];

};

struct game_client;

int send_packet(struct game_client* game, int packet_type, void* data_0, size_t size_0, void* data_1, size_t size_1, void* data_2, size_t size_2);

void init_networker(struct game_client* game);

void networker_thread_function(struct game_client* game);