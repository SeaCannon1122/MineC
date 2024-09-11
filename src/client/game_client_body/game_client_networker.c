#include "game_client_networker.h"
#include "game_client.h"


void init_networker(struct game_client* game) {
	game->networker.networking_state = NETWORK_INACTIVE;
	game->networker.requested_networking_state = NETWORK_INACTIVE;
}