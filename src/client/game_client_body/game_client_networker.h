#pragma once

enum networking_status {
	NETWORK_INACTIVE,
	NETWORK_CONNECTING, 
	NETWORK_CONNECTED,

};

enum networker_message {
	NETWORKER_MESSAGE_NULL_MESSAGE,
	NETWORKER_MESSAGE_CONNECTING,
	NETWORKER_MESSAGE_CONNECTION_FAILED,
	NETWORKER_MESSAGE_AUTHENTICATING,
	NETWORKER_MESSAGE_AUTHENTICATION_FAILED,
};

enum networker_request_type {
	NULL_REQUEST,
	CONNECT_TO_SERVER, 
	DISCONNECT_FROM_SERVER,
	SEND_GAME_PACKET,
};

struct game_client;

void init_networker(struct game_client* game);

void networker_thread(struct game_client* game);