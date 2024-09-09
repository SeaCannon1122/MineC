#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general/networking/client.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    client_init();

    void* client_handle = client_connect(SERVER_IP, SERVER_PORT);
    if (!client_handle) {
        fprintf(stderr, "Failed to connect to server.\n");
        client_cleanup();
        return EXIT_FAILURE;
    }

    char message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Enter a message to send to the server: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0'; // Remove newline character

    client_send(client_handle, message);

    int received = client_receive(client_handle, response, sizeof(response) - 1);
    if (received >= 0) {
        response[received] = '\0'; // Null-terminate the received message
        printf("Received response from server: %s\n", response);
    }
    else {
        fprintf(stderr, "Error receiving response from server.\n");
    }

    client_close(client_handle);
    client_cleanup();
    return EXIT_SUCCESS;
}
