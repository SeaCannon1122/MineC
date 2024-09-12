#include "game_client_networker.h"

#include <stdio.h>

#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"
#include "game_client.h"


void init_networker(struct game_client* game) {
	game->networker.network_handle = NULL;
	game->networker.status = NETWORK_INACTIVE;
	game->networker.message = NETWORKER_MESSAGE_NULL_MESSAGE;
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
}

void networker_thread(struct game_client* game) {

	while (game->running) {

		if (game->networker.request == CONNECT_TO_SERVER) {
			printf("[NETWORKER] connecting to client %s on port %d ...\n", game->networker.ip, game->networker.port);
			game->networker.message = NETWORKER_MESSAGE_CONNECTING;
			game->networker.network_handle = client_connect(game->networker.ip, game->networker.port, &game->networker.close_connection_flag);
			if (game->networker.network_handle == NULL) {
				game->networker.message = NETWORKER_MESSAGE_CONNECTION_FAILED;
				printf("[NETWORKER] Failed to connect to client %s on port %d\n", game->networker.ip, game->networker.port);
			}
			game->networker.close_connection_flag = false;
			game->networker.status = NETWORK_CONNECTING;
			game->networker.request = NULL_REQUEST;
		}

		sleep_for_ms(100);
	}

}