#include "game_server.h"

#include <stdio.h>
#include <string.h>

#include "general/resource_manager.h"

#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"

#include "game/networking_packets/networking_packets.h"


void new_game_server(struct game_server* game, char* resource_path) {
    game->resource_manager = new_resource_manager(resource_path);
    game->username_password_map = get_value_from_key(game->resource_manager, "auth").ptr;
    for (int i = 0; i < MAX_CLIENTS; i++) game->client_handles[i] = 0;
    game->currently_connected = 0;
}



void handle_client(struct game_server* game) {

    void* client_handle = game->client_handles[game->currently_connected - 1];

    int packet_type = -1;
    struct networking_packet_client_auth auth_packet;
    int recieved = 1;

    printf("waiting for auth packet\n");

    while (recieved == 1) {
        int recieve_state = server_receive(client_handle, &packet_type, sizeof(int));
        sleep_for_ms(1);
    }

    if (packet_type != NETWORKING_PACKET_CLIENT_AUTH) {
        server_close_client(client_handle);
        return;
    }
    recieved = 1;

    while (recieved == 1) {
        int recieve_state = server_receive(client_handle, &auth_packet, sizeof(struct networking_packet_client_auth));
        sleep_for_ms(1);
    }

    printf("recieved auth packet\n");

    if (!strcmp(auth_packet.password, get_value_from_key(game->resource_manager, auth_packet.username).ptr)) {
        int packet_type = NETWORKING_PACKET_LOGGED_IN;
        server_send(client_handle, &packet_type, sizeof(int));
        printf("auth success\n");
    }
    else {
        int packet_type = NETWORKING_PACKET_INVALID_PASSWORD;
        server_send(client_handle, &packet_type, sizeof(int));
        server_close_client(client_handle);
        printf("auth failed\n");
    }

    
}

void run_game_server(struct game_server* game) {

    bool interrupt = false;

    void* server_handle = server_init(8080);
    if (!server_handle) {
        printf("Failed to initialize server.\n");
        return;
    }

    printf("Server started on port 8080.\n");

    while (!get_key_state(KEY_ESCAPE)) {
        void* client_handle = server_accept(server_handle, &interrupt);
        if (!client_handle) {
            printf("Failed to accept client.\n");
            continue;
        }
        game->client_handles[game->currently_connected] = client_handle;
        game->currently_connected++;
        create_thread((void (*)(void*))handle_client, game);
        sleep_for_ms(1);
    }

    printf("server_closing\n");
    server_close(server_handle);
}

void delete_game_server(struct game_server* game) {

}