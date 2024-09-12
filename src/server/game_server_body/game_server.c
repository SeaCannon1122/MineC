#include "game_server.h"

#include <stdio.h>
#include <string.h>

#include "general/resource_manager.h"

#include "general/platformlib/platform.h"
#include "general/platformlib/networking.h"

#include "game/networking_packets/networking_packets.h"

void new_game_server(struct game_server* game, char* resource_path) {
    game->resource_manager = new_resource_manager(resource_path);
}

#define BUFFER_SIZE 4096

void handle_packet(int packet_type, char* packet) {

    switch (packet_type) {
    case NETWORKING_PACKET_MESSAGE_SMALL: {
        printf("Received small_message: %s\n", packet); break;
    }
    case NETWORKING_PACKET_MESSAGE_LARGE: {
        printf("Received large message: %s\n", packet); break;
    }
    default:
        printf("Unknown packet type.\n");
        break;
    }
}

void* handle_client(void* client_handle) {
    char buffer[BUFFER_SIZE];

    while (1) {
        int packet_type = -1;
        int bytes_received = server_receive(client_handle, &packet_type, 4);
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            break;
        }


        printf("recived packet of type: %d\n", packet_type);

        switch (packet_type) {
        case NETWORKING_PACKET_MESSAGE_SMALL: server_receive(client_handle, buffer, sizeof(struct networking_packet_message_small)); handle_packet(NETWORKING_PACKET_MESSAGE_SMALL, buffer); break;
        case NETWORKING_PACKET_MESSAGE_LARGE: server_receive(client_handle, buffer, sizeof(struct networking_packet_message_large)); handle_packet(NETWORKING_PACKET_MESSAGE_LARGE, buffer); break;
        }

        
    }

    server_close(client_handle);
    return NULL;
}

void run_game_server(struct game_server* game) {

    bool interrupt = false;

    void* server_handle = server_init(8080);
    if (!server_handle) {
        printf("Failed to initialize server.\n");
        return 1;
    }

    printf("Server started on port 8080.\n");

    while (!get_key_state(KEY_ESCAPE)) {
        void* client_handle = server_accept(server_handle, &interrupt);
        if (!client_handle) {
            printf("Failed to accept client.\n");
            continue;
        }

        create_thread(handle_client, client_handle);
    }

    printf("server_closing\n");
    server_close(server_handle);
    return 0;
}

void delete_game_server(struct game_server* game) {

}