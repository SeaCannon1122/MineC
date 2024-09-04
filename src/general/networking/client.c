#include "client.h"

#include "networking.h"

#if defined(_WIN32)

static int initialize_winsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 0;
    }
    return 1;
}

void* client_connect(const char* ip, int port) {
    if (!initialize_winsock()) return NULL;

    struct NetworkHandle* handle = (struct NetworkHandle*)malloc(sizeof(struct NetworkHandle));
    if (!handle) return NULL;

    handle->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (handle->socket_fd == INVALID_SOCKET) {
        printf("Socket creation failed: %ld\n", WSAGetLastError());
        free(handle);
        WSACleanup();
        return NULL;
    }

    handle->address.sin_family = AF_INET;
    handle->address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &handle->address.sin_addr) <= 0) {
        printf("Invalid address\n");
        closesocket(handle->socket_fd);
        free(handle);
        WSACleanup();
        return NULL;
    }

    if (connect(handle->socket_fd, (struct sockaddr*)&handle->address, sizeof(handle->address)) < 0) {
        printf("Connection failed\n");
        closesocket(handle->socket_fd);
        free(handle);
        WSACleanup();
        return NULL;
    }

    return handle;
}

int client_receive(void* handle, char* buffer, size_t length) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    return recv(h->socket_fd, buffer, length, 0);
}

void client_send(void* handle, const char* message) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    send(h->socket_fd, message, (int)strlen(message), 0);
}

void client_close(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    closesocket(h->socket_fd);
    free(h);
    WSACleanup();
}

#elif defined(__linux__)

void* client_connect(const char* ip, int port) {
    struct NetworkHandle* handle = (struct NetworkHandle*)malloc(sizeof(struct NetworkHandle));
    if (!handle) return NULL;

    handle->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (handle->socket_fd < 0) {
        perror("Socket creation failed");
        free(handle);
        return NULL;
    }

    handle->address.sin_family = AF_INET;
    handle->address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &handle->address.sin_addr) <= 0) {
        perror("Invalid address");
        close(handle->socket_fd);
        free(handle);
        return NULL;
    }

    if (connect(handle->socket_fd, (struct sockaddr*)&handle->address, sizeof(handle->address)) < 0) {
        perror("Connection failed");
        close(handle->socket_fd);
        free(handle);
        return NULL;
    }

    return handle;
}

int client_receive(void* handle, char* buffer, size_t length) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    return read(h->socket_fd, buffer, length);
}

void client_send(void* handle, const char* message) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    send(h->socket_fd, message, strlen(message), 0);
}

void client_close(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    close(h->socket_fd);
    free(h);
}

#endif