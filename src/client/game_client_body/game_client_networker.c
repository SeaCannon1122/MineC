#include "game_client_networker.h"

#include <stdio.h>

#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"
#include "general/utils.h"
#include "game/networking_packets/networking_packets.h"
#include "game_client.h"
#include "general/logging.h"

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
	}
	game->networker.status = NETWORK_INACTIVE;

	if (game->networker.network_handle != NULL) {
		close_connection(game->networker.network_handle);
	}
	game->networker.network_handle = NULL;

	
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
}

void networker_thread(struct game_client* game) {

	log_message(game->debug_log_file, "[NETWORKER] Started Networking Thread\n");

	while (game->running) {

		if (game->networker.status == NETWORK_CONNECTED) {
			if (game->networker.close_connection_flag) {
				game->networker.message = NETWORKER_MESSAGE_DISCONNECTED;
				networker_disconnect_and_cleanup(game);
				log_message(game->debug_log_file, "[NETWORKER] Disconnected from server % s: % u\n", game->networker.ip, game->networker.port);
			}
			else if (!is_connected(game->networker.network_handle)) {
				game->networker.message = NETWORKER_MESSAGE_CONNECTION_LOST;
				networker_disconnect_and_cleanup(game);
				log_message(game->debug_log_file, "[NETWORKER] Disconnected from server %s:%u\n", game->networker.ip, game->networker.port);
			}
			else {
				int packet_type;
				if (receive_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag, game->constants.packet_awaiting_timeout) == 0) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

			}
		}

		else if (game->networker.request == CONNECT_TO_SERVER) {
			game->networker.request = NULL_REQUEST;
			game->networker.status = NETWORK_CONNECTING;

			if (game->networker.network_handle == NULL) {
				log_message(game->debug_log_file, "[NETWORKER] Connecting to server %s:%u ...\n", game->networker.ip, game->networker.port);
				game->networker.message = NETWORKER_MESSAGE_CONNECTING;
				
				game->networker.network_handle = client_connect(game->networker.ip, game->networker.port, &game->networker.close_connection_flag, game->constants.client_connection_timeout);

				if (game->networker.network_handle == NULL) {
					game->networker.message = NETWORKER_MESSAGE_CONNECTION_FAILED;
					log_message(game->debug_log_file, "[NETWORKER] Failed to connect to server %s:%u\n", game->networker.ip, game->networker.port);
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

				if (receive_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag, game->constants.packet_awaiting_timeout) <= 0) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

				if (game->networker.close_connection_flag) {
					networker_disconnect_and_cleanup(game);
					continue;
				}

				if (packet_type == NETWORKING_PACKET_LOGGED_IN) {

					if (receive_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag, game->constants.packet_awaiting_timeout) <= 0) {
						networker_disconnect_and_cleanup(game);
						continue;
					}
					struct networking_packet_server_settings server_settings;
					if (receive_data(game->networker.network_handle, &server_settings, sizeof(struct networking_packet_server_settings), &game->networker.close_connection_flag, game->constants.packet_awaiting_timeout) <= 0) {
						networker_disconnect_and_cleanup(game);
						continue;
					}
					game->server_settings.max_message_length = server_settings.max_message_length;
					game->server_settings.max_render_distance = server_settings.max_render_distance;
					log_message(game->debug_log_file, "[NETWORKER] Connected to server %s:%u\n", game->networker.ip, game->networker.port);
					log_message(game->debug_log_file, "[NETWORKER] Max render distance %d\n", game->server_settings.max_render_distance);
					log_message(game->debug_log_file, "[NETWORKER] Max message length %d\n", game->server_settings.max_message_length);
					game->networker.message = NETWORKER_MESSAGE_CONNECTED;
					game->networker.status = NETWORK_CONNECTED;
				}
				else if (packet_type == NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN) {
					game->networker.message = NETWORKER_MESSAGE_NOT_AUTHORIZED;
					log_message(game->debug_log_file, "[NETWORKER] Not authorized to join server %s:%u\n", game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}
				else if (packet_type == NETWORKING_PACKET_INVALID_PASSWORD) {
					game->networker.message = NETWORKER_MESSAGE_WRONG_PASSWORD;
					log_message(game->debug_log_file, "[NETWORKER] Incorrect password for user %s on server %s:%u\n", game->networker.username, game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}
				else if (packet_type == NETWORKING_PACKET_SERVER_FULL) {
					game->networker.message = NETWORKER_MESSAGE_SERVER_FULL;
					log_message(game->debug_log_file, "[NETWORKER] Server %s:%u is full\n", game->networker.ip, game->networker.port);
					networker_disconnect_and_cleanup(game);
				}

				
			}
	
		}

		sleep_for_ms(100);
	}

	networker_disconnect_and_cleanup(game);

}