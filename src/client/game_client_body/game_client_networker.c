#include "game_client_networker.h"

#include <stdio.h>

#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"
#include "game/networking_packets/networking_packets.h"
#include "game_client.h"


void init_networker(struct game_client* game) {
	game->networker.network_handle = NULL;
	game->networker.status = NETWORK_INACTIVE;
	game->networker.message = NETWORKER_MESSAGE_NULL_MESSAGE;
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
}

void networker_disconnect_and_cleanup(struct game_client* game) {
	if (game->networker.status == NETWORK_CONNECTED) {
		int packet_type = NETWORKING_PACKET_DISCONNECT;
		send_data(game->networker.network_handle, &packet_type, sizeof(int));
		game->networker.status = NETWORK_INACTIVE;
	}

	if (game->networker.network_handle != NULL) {
		close_connection(game->networker.network_handle);
	}
	game->networker.network_handle = NULL;

	
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
}

void networker_thread(struct game_client* game) {

	while (game->running) {

		if (game->networker.status == NETWORK_CONNECTED) {
			if (game->networker.close_connection_flag) {
				networker_disconnect_and_cleanup(game);
				printf("[NETWORKER] Disconnected from server %s:%u\n", game->networker.ip, game->networker.port);
			}
			else if (!is_connected(game->networker.network_handle)) {
				game->networker.message = NETWORKER_MESSAGE_DISCONNECTED;
				networker_disconnect_and_cleanup(game);
				printf("[NETWORKER] Disconnected from server %s:%u\n", game->networker.ip, game->networker.port);
			}
		}

		else if (game->networker.request == CONNECT_TO_SERVER) {
			game->networker.request = NULL_REQUEST;

			if (game->networker.network_handle == NULL) {
				printf("[NETWORKER] Connecting to server %s:%u ...\n", game->networker.ip, game->networker.port);
				game->networker.message = NETWORKER_MESSAGE_CONNECTING;
				
				game->networker.network_handle = client_connect(game->networker.ip, game->networker.port, &game->networker.close_connection_flag, CLIENT_CONNECTION_TIMEOUT_TIME);

				if (game->networker.network_handle == NULL) {
					game->networker.message = NETWORKER_MESSAGE_CONNECTION_FAILED;
					printf("[NETWORKER] Failed to connect to server %s:%u\n", game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
					continue;
				}

				if (game->networker.close_connection_flag) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

				game->networker.message = NETWORKER_MESSAGE_AUTHENTICATING;

				struct networking_packet_client_auth auth_packet;
				for (int i = 0; i < MAX_USERNAME_LENGTH; i++) auth_packet.username[i] = game->networker.username[i];
				for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) auth_packet.password[i] = game->networker.password[i];

				int packet_type = NETWORKING_PACKET_CLIENT_AUTH;
				send_data(game->networker.network_handle, &packet_type, sizeof(int));
				send_data(game->networker.network_handle, &auth_packet, sizeof(struct networking_packet_client_auth));

				if (receive_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag, 1000) <= 0) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

				if (game->networker.close_connection_flag) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

				if (packet_type == NETWORKING_PACKET_LOGGED_IN) {

					game->networker.message = NETWORKER_MESSAGE_CONNECTED;
					game->networker.status = NETWORK_CONNECTED;
					printf("[NETWORKER] Connected to server %s:%u\n", game->networker.ip, game->networker.port);
				}
				else if (packet_type == NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN) {
					game->networker.message = NETWORKER_MESSAGE_NOT_AUTHORIZED;
					printf("[NETWORKER] Not authorized to join server %s:%u\n", game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}
				else if (packet_type == NETWORKING_PACKET_INVALID_PASSWORD) {
					game->networker.message = NETWORKER_MESSAGE_WRONG_PASSWORD;
					printf("[NETWORKER] Incorrect password for user %s on server %s:%u\n", game->networker.username, game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}
				else if (packet_type == NETWORKING_PACKET_SERVER_FULL) {
					game->networker.message = NETWORKER_MESSAGE_SERVER_FULL;
					printf("[NETWORKER] Server %s:%u is full\n", game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}

				
			}

			
			
		}



		sleep_for_ms(100);
	}

}