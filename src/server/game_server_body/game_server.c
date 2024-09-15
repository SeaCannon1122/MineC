#include "game_server.h"

#include <stdio.h>
#include <string.h>

#include "general/resource_manager.h"

#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"

#include "game/networking_packets/networking_packets.h"


void new_game_server(struct game_server* game, char* resource_path) {
    game->resource_manager = new_resource_manager(resource_path);
    game->username_password_map = get_value_from_key(game->resource_manager, "passwords").ptr;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        game->clients[i].client_handle = NULL;
        for (int j = 0; j < 22 + 1; j++) game->clients[i].ip_address[j] = '\0';
        for (int j = 0; j < MAX_USERNAME_LENGTH + 1; j++) game->clients[i].username[j] = '\0';
    }
    game->currently_connected = 0;
}



void run_game_server(struct game_server* game) {

    show_console_window();

    void* server_handle = server_init(8080);
    if (!server_handle) {
        printf("Failed to initialize server.\n");
        return;
    }

    printf("Server started on port 8080.\n");

    while (!get_key_state(KEY_ESCAPE)) {
        void* client_handle = server_accept(server_handle, NULL, 100);
        if (client_handle) {
            
            int packet_type = -1;
            struct networking_packet_client_auth auth_packet;
            int recieved;

            recieved = receive_data(client_handle, &packet_type, sizeof(int), NULL, -1);

            if (packet_type != NETWORKING_PACKET_CLIENT_AUTH) {
                close_connection(client_handle);
                continue;
            }

            recieved = receive_data(client_handle, &auth_packet, sizeof(struct networking_packet_client_auth), NULL, -1);
            char* password = get_value_from_key(game->username_password_map, auth_packet.username).ptr;

            if (password == NULL) {
                int packet_type = NETWORKING_PACKET_NOT_AUTHORIZED_TO_JOIN;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
                printf("%s: not autherized to join\n", auth_packet.username);
            }
            else if (strcmp(auth_packet.password, password)) {
                int packet_type = NETWORKING_PACKET_INVALID_PASSWORD;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
                printf("%s: incorrect password\n", auth_packet.username);
            }
            else {
                int packet_type = NETWORKING_PACKET_LOGGED_IN;
                send_data(client_handle, &packet_type, sizeof(int));
                for (int i = 0; i < MAX_CLIENTS; i++) if (game->clients[i].client_handle == NULL) {
                    game->clients[i].client_handle = client_handle;
                    get_ip_address_and_port(client_handle, game->clients[i].ip_address, &game->clients[i].port);
                    for (int j = 0; auth_packet.username[j] != '\0'; j++) game->clients[i].username[j] = auth_packet.username[j];
                    printf("%s from %s:%u connected to the server\n", game->clients[i].username, game->clients[i].ip_address, game->clients[i].port);
                    break;
                }
            }

        }

        for (int i = 0; i < MAX_CLIENTS; i++) if (game->clients[i].client_handle != NULL) {
            if (!is_connected(game->clients[i].client_handle)) {
                close_connection(game->clients[i].client_handle);
                printf("%s from %s:%u disconnected from the server\n", game->clients[i].username, game->clients[i].ip_address, game->clients[i].port);
                game->clients[i].client_handle = NULL;
            } 
        }

        sleep_for_ms(1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) if (game->clients[i].client_handle != NULL) close_connection(game->clients[i].client_handle);

    printf("\n\nserver_closing\n");
    server_close(server_handle);
}

void delete_game_server(struct game_server* game) {

}