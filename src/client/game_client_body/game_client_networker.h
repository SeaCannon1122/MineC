#pragma once

enum networking_states {
	NETWORK_INACTIVE,
	NETWORK_CONNECTING,

};

void init_networker(struct game_client* game);