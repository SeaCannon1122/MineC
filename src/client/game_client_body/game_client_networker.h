#pragma once

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

struct game_client;

void init_networker(struct game_client* game);

void networker_thread(struct game_client* game);