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

void send_packet(struct game_client* game, int packet_type, void* packet) {
	size_t packet_size = 0;
	
	switch (packet_type)
	{
		
	case NETWORKING_PACKET_CLIENT_AUTH: packet_size = sizeof(struct networking_packet_client_auth); break;
	case NETWORKING_PACKET_DISCONNECT: packet_size = 0;

	default: packet_size = 0;
	}

	send_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag);
	if(packet_size > 0) send_data(game->networker.network_handle, packet, packet_size, &game->networker.close_connection_flag);
}

void networker_disconnect_and_cleanup(struct game_client* game) {
	if (game->networker.status == NETWORK_CONNECTED) {
		send_packet(game, NETWORKING_PACKET_DISCONNECT, NULL);
		game->networker.status = NETWORK_INACTIVE;
	}

	if (game->networker.network_handle != NULL) close_connection(game->networker.network_handle);
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
				game->networker.status = NETWORK_INACTIVE;
				game->networker.network_handle = NULL;
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
				}

				if (game->networker.close_connection_flag) networker_disconnect_and_cleanup(game);

				game->networker.message = NETWORKER_MESSAGE_AUTHENTICATING;

				struct networking_packet_client_auth auth_packet;
				for (int i = 0; i < MAX_USERNAME_LENGTH; i++) auth_packet.username[i] = game->networker.username[i];
				for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) auth_packet.password[i] = game->networker.password[i];

				int packet_type = NETWORKING_PACKET_CLIENT_AUTH;
				send_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag);
				send_data(game->networker.network_handle, &auth_packet, sizeof(struct networking_packet_client_auth), &game->networker.close_connection_flag);
				
				//client_receive()


				game->networker.status = NETWORK_CONNECTED;
				printf("[NETWORKER] Connected to server %s:%u\n", game->networker.ip, game->networker.port);
			}

			
			
		}



		sleep_for_ms(100);
	}

}