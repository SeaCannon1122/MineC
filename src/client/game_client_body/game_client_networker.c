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
	game->networker.request = NULL_REQUEST;
	game->networker.close_connection_flag = false;
	game->networker.status_updates_missed = 0;
	game->networker.next_packet_type = -1;
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

	log_message(game->debug_log_file, "[NETWORKER] Started Networking Thread");

	while (game->running) {

		if (game->networker.request == CONNECT_TO_SERVER) {

			game->networker.request = NULL_REQUEST;
			game->networker.status = NETWORK_CONNECTING;

			log_message(game->debug_log_file, "[NETWORKER] Connecting to server %s:%u ...", game->networker.ip, game->networker.port);
			parse_string("Connecting ...", game->game_menus.connection_waiting_menu.networking_message);

			game->networker.network_handle = client_connect(game->networker.ip, game->networker.port, &game->networker.close_connection_flag, game->constants.client_connection_timeout);

			if (game->networker.network_handle == NULL) {
				parse_string("Connection failed", game->game_menus.connection_waiting_menu.networking_message);
				log_message(game->debug_log_file, "[NETWORKER] Failed to connect to server %s:%u", game->networker.ip, game->networker.port);
				networker_disconnect_and_cleanup(game);
				continue;
			}

			parse_string("Authenticating ...", game->game_menus.connection_waiting_menu.networking_message);

			struct networking_packet_client_auth auth_packet;
			for (int i = 0; i < MAX_USERNAME_LENGTH; i++) auth_packet.username[i] = game->networker.username[i];
			for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) auth_packet.password[i] = game->networker.password[i];

			int packet_type = NETWORKING_PACKET_CLIENT_AUTH;
			send_data(game->networker.network_handle, &packet_type, sizeof(int));
			send_data(game->networker.network_handle, &auth_packet, sizeof(struct networking_packet_client_auth));

			if (receive_data(game->networker.network_handle, &packet_type, sizeof(int), &game->networker.close_connection_flag, game->constants.packet_awaiting_timeout) <= 0) {
				parse_string("Connection lost", game->game_menus.connection_waiting_menu.networking_message);

				networker_disconnect_and_cleanup(game);
				continue;
			}

			if (packet_type == NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN) {
				parse_string("You are not permitted to join this server", game->game_menus.connection_waiting_menu.networking_message);
				log_message(game->debug_log_file, "[NETWORKER] Not authorized to join server %s:%u", game->networker.ip, game->networker.port);
				networker_disconnect_and_cleanup(game);
			}
			else if (packet_type == NETWORKING_PACKET_INVALID_PASSWORD) {
				parse_string("Wrong password", game->game_menus.connection_waiting_menu.networking_message);
				log_message(game->debug_log_file, "[NETWORKER] Incorrect password for user %s on server %s:%u", game->networker.username, game->networker.ip, game->networker.port);
				networker_disconnect_and_cleanup(game);
			}
			else if (packet_type == NETWORKING_PACKET_SERVER_FULL) {
				parse_string("Server at full capacity", game->game_menus.connection_waiting_menu.networking_message);
				log_message(game->debug_log_file, "[NETWORKER] Server %s:%u is full", game->networker.ip, game->networker.port);
				networker_disconnect_and_cleanup(game);
			}

			else if (packet_type == NETWORKING_PACKET_LOGGED_IN) {

				parse_string("Connected successfully", game->game_menus.connection_waiting_menu.networking_message);
				game->networker.status = NETWORK_CONNECTED;

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
				log_message(game->debug_log_file, "[NETWORKER] Connected to server %s:%u", game->networker.ip, game->networker.port);
				log_message(game->debug_log_file, "[NETWORKER] Max render distance %d", game->server_settings.max_render_distance);
				log_message(game->debug_log_file, "[NETWORKER] Max message length %d", game->server_settings.max_message_length);
				
				parse_string(" ", game->game_menus.connection_waiting_menu.networking_message);

				while (game->running) {
					sleep_for_ms(100);

					if (game->networker.close_connection_flag) {

						if(game->networker.next_packet_type == -1) receive_data(game->networker.network_handle, &game->networker.next_packet_type, sizeof(int), NULL, 0);

						if (game->networker.next_packet_type != -1) {
							switch (game->networker.next_packet_type) {
							case NETWORKING_PACKET_KICK: {

							}break;
							}
						}

						log_message(game->debug_log_file, "[NETWORKER] Disconnected from server % s: % u", game->networker.ip, game->networker.port);
						parse_string("Disconnected", game->game_menus.connection_waiting_menu.networking_message);
						break;
					}

				}

				networker_disconnect_and_cleanup(game);
				
			}



		}


		sleep_for_ms(100);
	}

}