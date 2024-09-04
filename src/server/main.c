#include <stdio.h>

#include "general/networking/server.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    void* server = server_init(PORT);
    if (server == NULL) {
        printf("Failed to initialize server\n");
        return -1;
    }

    server_listen(server);
    printf("Server listening on port %d\n", PORT);

    server_accept(server);
    printf("Client connected\n");

    char buffer[BUFFER_SIZE] = { 0 };
    int bytes_received = server_receive(server, buffer, BUFFER_SIZE);
    if (bytes_received > 0) {
        printf("Received: %s\n", buffer);
        server_send(server, "Hello from server");
    }

    server_close(server);
    return 0;
}