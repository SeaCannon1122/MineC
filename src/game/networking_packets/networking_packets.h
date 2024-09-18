#pragma once

#include "game/game_constants.h"

enum networking_packet_type {
	NETWORKING_PACKET_MESSAGE,
	NETWORKING_PACKET_CLIENT_AUTH,
	NETWORKING_PACKET_SERVER_FULL,
	NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN,
	NETWORKING_PACKET_INVALID_PASSWORD,
	NETWORKING_PACKET_LOGGED_IN,
	NETWORKING_PACKET_SERVER_SETTINGS,
	NETWORKING_PACKET_DISCONNECT,
};


struct networking_packet_client_auth {
	char username[MAX_USERNAME_LENGTH + 1];
	char password[MAX_PASSWORD_LENGTH + 1];
};

struct networking_packet_server_settings {
	int max_render_distance;
	int max_message_length;
};