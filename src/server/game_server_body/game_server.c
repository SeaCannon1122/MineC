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
}

struct accept_client_args {
    struct game_server* game;
    void* client_handle;
};

void client_auth_thread(struct game_server* game) {
    while (game->running) {
        void* client_handle;
        for (int i = 0; (client_handle = server_accept(game->server_handle)) != NULL && i < 10; i++) {

            int client_index = 0;
            for (; client_index < MAX_CLIENTS && game->clients[client_index].client_handle != NULL; client_index++);

            int packet_type = -1;
            struct networking_packet_client_auth auth_packet;
            int recieved;

            if (receive_data(client_handle, &packet_type, sizeof(int), NULL, 3000) == -2) {
                close_connection(client_handle);
                return;
            }

            if (packet_type != NETWORKING_PACKET_CLIENT_AUTH) {
                close_connection(client_handle);
                return;
            }

            if (receive_data(client_handle, &auth_packet, sizeof(struct networking_packet_client_auth), NULL, 3000) == -2) {
                close_connection(client_handle);
                return;
            }

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
            else if (client_index == MAX_CLIENTS) {
                int packet_type = NETWORKING_PACKET_SERVER_FULL;
                send_data(client_handle, &packet_type, sizeof(int));
                close_connection(client_handle);
            }
            else {
                int packet_type = NETWORKING_PACKET_LOGGED_IN;
                send_data(client_handle, &packet_type, sizeof(int));
                game->clients[client_index].client_handle = client_handle;
                get_ip_address_and_port(client_handle, game->clients[client_index].ip_address, &game->clients[client_index].port);
                int j = 0;
                for (; auth_packet.username[j] != '\0'; j++) game->clients[client_index].username[j] = auth_packet.username[j];
                game->clients[client_index].username[j] = auth_packet.username[j];
                printf("%s from %s:%u connected to the server\n", game->clients[client_index].username, game->clients[client_index].ip_address, game->clients[client_index].port);
            }
        }

        sleep_for_ms(100);
    }

    printf("stopped accepting new clients\n");

}

void disconnect_client(struct game_server* game, int client_index) {
    close_connection(game->clients[client_index].client_handle);
    printf("%s from %s:%u disconnected from the server\n", game->clients[client_index].username, game->clients[client_index].ip_address, game->clients[client_index].port);
    game->clients[client_index].client_handle = NULL;
}

void run_game_server(struct game_server* game) {
    printf("Server starting...\n");
    show_console_window();

    game->server_handle = server_init(8080);
    if (!game->server_handle) {
        printf("Failed to initialize server.\n");
        return;
        exit(1);
    }
    printf("Server started on port 8080.\n");

    game->running = true;

    //create_thread();

    create_thread((void (*) (void*))client_auth_thread, game);

    while (game->running) {
        
        

        for (int i = 0; i < MAX_CLIENTS; i++) if (game->clients[i].client_handle != NULL) {

            for (int packet_count = 0; packet_count < 5; packet_count++) {
                int packet_type = -1;
                if(receive_data(game->clients[i].client_handle, &packet_type, sizeof(int), NULL, 0) == 0) {
                    disconnect_client(game, i);
                    continue;
                }

                if (packet_type == NETWORKING_PACKET_DISCONNECT) {
                    disconnect_client(game, i);
                    continue;
                }

            }

        }

        sleep_for_ms(10);
        if (get_key_state(KEY_ESCAPE)) game->running == false;
    }

    join_thread()

    for (int i = 0; i < MAX_CLIENTS; i++) if (game->clients[i].client_handle != NULL) close_connection(game->clients[i].client_handle);

    printf("\n\nserver_closing\n");
    server_close(game->server_handle);
}

void delete_game_server(struct game_server* game) {

}