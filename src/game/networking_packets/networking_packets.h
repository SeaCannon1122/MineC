#pragma once

#include "game/game_constants.h"

enum networking_packet_type {
	NETWORKING_PACKET_CLIENT_AUTH,
	NETWORKING_PACKET_SERVER_FULL,
	NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN,
	NETWORKING_PACKET_INVALID_PASSWORD,
	NETWORKING_PACKET_LOGGED_IN,
	NETWORKING_PACKET_SERVER_SETTINGS,
	NETWORKING_PACKET_DISCONNECT,
	NETWORKING_PACKET_KICK,

	
	NETWORKING_PACKET_CLOCK,
	NETWORKING_PACKET_MESSAGE,
};


struct networking_packet_client_auth {
	char username[MAX_USERNAME_LENGTH + 1];
	char password[MAX_PASSWORD_LENGTH + 1];
};

struct networking_packet_server_settings {
	int max_render_distance;
};

struct networking_packet_clock {
	long long tick;
};

struct networking_packet_kick {
	char kick_message[CONECTION_CLOSE_MESSAGE_LENGTH + 1];
};

//networking packet message
//size_t size_with_null_termination
//char[] data