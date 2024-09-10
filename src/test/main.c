#include "general/platformlib/networking.h"
#include <stdio.h>
#include <string.h>
#include "general/platformlib/platform.h"
#include "general/utils.h"

#include "game/networking_packets/networking_packets.h"


void send_packet(void* client_handle, int packet_type, const char* data) {

    int packet_size = 0;

    switch (packet_type) {
    case NETWORKING_PACKET_MESSAGE_SMALL: packet_size = sizeof(struct networking_packet_message_small); break;
    case NETWORKING_PACKET_MESSAGE_LARGE: packet_size = sizeof(struct networking_packet_message_large); break;
    }

    char buffer[4069 + 4];
    memcpy(buffer, &packet_type, 4); // Set packet type
    memcpy(buffer + 4, data, packet_size); // Set packet data

    client_send(client_handle, buffer, 4 + packet_size);
}

void* client_thread(void* client_handle) {
    char buffer[4096];

    int counter = 0;

    char recieve_buffer[4069];

    for (; 1; counter++) {
        // Example packet sending
        
        snprintf(buffer, sizeof(buffer), "small message packet %d", counter);

        send_packet(client_handle, NETWORKING_PACKET_MESSAGE_SMALL, buffer);

        sleep_for_ms(1000);  // Send every 2 seconds
    }

    client_close(client_handle);
    return NULL;
}

int main() {
    networking_init();

    void* client_handle = client_connect("127.0.0.1", 8080);
    if (!client_handle) {
        printf("Failed to connect to server.\n");
        return 1;
    }

    printf("Connected to server.\n");

    create_thread(client_thread, client_handle);

    while (1) {
        sleep_for_ms(5000);  // Keep the main thread alive
    }

    client_close(client_handle);
    networking_exit();
    return 0;
}