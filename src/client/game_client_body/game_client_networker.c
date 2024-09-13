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

	client_send(game->networker.network_handle, &packet_type, sizeof(int));
	if(packet_size > 0) client_send(game->networker.network_handle, packet, packet_size);
}

void networker_disconnect_and_cleanup(struct game_client* game) {
	if (game->networker.status == NETWORK_CONNECTED) {
		send_packet(game, NETWORKING_PACKET_DISCONNECT, NULL); // send disconnect package
		//wait while recieve package
	}

	if (game->networker.network_handle != NULL) client_close(game->networker.network_handle);

	game->networker.status = NETWORK_INACTIVE;
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
}

void networker_thread(struct game_client* game) {

	while (game->running) {

		if (game->networker.request == CONNECT_TO_SERVER) {
			game->networker.request = NULL_REQUEST;

			if (game->networker.network_handle == NULL) {
				printf("[NETWORKER] connecting to client %s on port %d ...\n", game->networker.ip, game->networker.port);
				game->networker.message = NETWORKER_MESSAGE_CONNECTING;
				
				game->networker.network_handle = client_connect(game->networker.ip, game->networker.port, &game->networker.close_connection_flag);

				if (game->networker.network_handle == NULL) {
					game->networker.message = NETWORKER_MESSAGE_CONNECTION_FAILED;
					printf("[NETWORKER] Failed to connect to client %s on port %d\n", game->networker.ip, game->networker.port);
					goto _disconnect_cleanup;
				}

				if (game->networker.close_connection_flag) goto _disconnect_cleanup;

				game->networker.message = NETWORKER_MESSAGE_AUTHENTICATING;

				struct networking_packet_client_auth auth_packet;
				for (int i = 0; i < MAX_USERNAME_LENGTH; i++) auth_packet.username[i] = game->networker.username[i];
				for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) auth_packet.password[i] = game->networker.password[i];

				int packet_type = NETWORKING_PACKET_CLIENT_AUTH;
				client_send(game->networker.network_handle, &packet_type, sizeof(int));
				client_send(game->networker.network_handle, &auth_packet, sizeof(struct networking_packet_client_auth));
				



				
			}
			
			
			
			
			
			
			continue;
		_disconnect_cleanup:
			networker_disconnect_and_cleanup(game);
		
		}



		sleep_for_ms(100);
	}

}