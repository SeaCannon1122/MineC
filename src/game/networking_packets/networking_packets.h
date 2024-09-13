#pragma once

#include "game/game_constants.h"

enum networking_packet_type {
	NETWORKING_PACKET_MESSAGE_SMALL,
	NETWORKING_PACKET_MESSAGE_LARGE,
	NETWORKING_PACKET_CLIENT_AUTH,
	NETWORKING_PACKET_INVALID_PASSWORD,
	NETWORKING_PACKET_LOGGED_IN,
	NETWORKING_PACKET_DISCONNECT,
};


struct networking_packet_client_auth {
	char username[MAX_USERNAME_LENGTH + 1];
	char password[MAX_PASSWORD_LENGTH + 1];
};

struct networking_packet_message_small {
	char message[256];
};

struct networking_packet_message_large {
	char message[4069];
};