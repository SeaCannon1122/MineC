#pragma once

#ifndef GAME_H
#define GAME_H

#define MAX_CONNECTION_STATE_MESSAGE_LENGTH 256

#define GAME_MAX_USERNAME_LENGTH 64
#define GAME_MAX_PASSWORD_LENGTH 64

enum block_type_ {
	BLOCK_AIR = 1,
	BLOCK_DIRT,
	BLOCK_GRASS,
	BLOCK_STONE,
	BLOCK_MAX,
};

struct entity {
	float x;
	float y;
	float z;

	float pitch;
	float roll;
	float yaw;

	uint16_t type;

	struct entity_data {
		uint8_t gravity;
	};
};

struct game_char_ {
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t value;
	uint8_t masks;
	uint8_t font;
};

enum game_font_types {
	GAME_FONT_DEFAULT,
	GAME_FONT_WIDE,
	GAME_FONT_RECTANGULAR
};

struct networking_packet_header {
	uint32_t type;
	uint32_t size;
};

enum networking_packet_type {
	NETWORKING_PACKET_CLIENT_AUTH,
	NETWORKING_PACKET_CONNECTION_STATE,

	NETWORKING_PACKET_TICK,
	NETWORKING_PACKET_CHAT_MESSAGE_TO_SERVER,
	NETWORKING_PACKET_CHAT_MESSAGE_FROM_SERVER,

};

struct networking_packet_client_auth {
	uint32_t username[GAME_MAX_USERNAME_LENGTH];
	uint32_t username_length;
	uint32_t password[GAME_MAX_PASSWORD_LENGTH];
	uint32_t password_length;
};

struct networking_packet_clock {
	long long tick;
};

struct networking_packet_connection_state {
	uint32_t state;
	uint32_t message_length;
	uint32_t message[];
};

struct networking_packet_chat_message_to_server {
	uint32_t message_length;
	uint32_t message[];
};

struct networking_packet_chat_message_from_server {
	uint32_t message_length;
	struct game_char message[];
};

#endif // !GAME_H
