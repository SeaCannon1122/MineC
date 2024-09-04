#include "server.h"

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

void* server_init(int port) {
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
    handle->address.sin_addr.s_addr = INADDR_ANY;
    handle->address.sin_port = htons(port);

    if (bind(handle->socket_fd, (struct sockaddr*)&handle->address, sizeof(handle->address)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(handle->socket_fd);
        free(handle);
        WSACleanup();
        return NULL;
    }

    return handle;
}

void server_listen(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    if (listen(h->socket_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(h->socket_fd);
        free(h);
        WSACleanup();
    }
}

void server_accept(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    int addrlen = sizeof(h->address);
    SOCKET new_socket = accept(h->socket_fd, (struct sockaddr*)&h->address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
        closesocket(h->socket_fd);
        free(h);
        WSACleanup();
    }
    else {
        h->socket_fd = new_socket;
    }
}

int server_receive(void* handle, char* buffer, size_t length) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    return recv(h->socket_fd, buffer, length, 0);
}

void server_send(void* handle, const char* message) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    send(h->socket_fd, message, (int)strlen(message), 0);
}

void server_close(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    closesocket(h->socket_fd);
    free(h);
    WSACleanup();
}

#elif defined(__linux__)

void* server_init(int port) {
    struct NetworkHandle* handle = (struct NetworkHandle*)malloc(sizeof(struct NetworkHandle));
    if (!handle) return NULL;

    handle->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (handle->socket_fd == 0) {
        perror("Socket creation failed");
        free(handle);
        return NULL;
    }

    handle->address.sin_family = AF_INET;
    handle->address.sin_addr.s_addr = INADDR_ANY;
    handle->address.sin_port = htons(port);

    if (bind(handle->socket_fd, (struct sockaddr*)&handle->address, sizeof(handle->address)) < 0) {
        perror("Bind failed");
        close(handle->socket_fd);
        free(handle);
        return NULL;
    }

    return handle;
}

void server_listen(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    if (listen(h->socket_fd, 3) < 0) {
        perror("Listen failed");
        close(h->socket_fd);
        free(h);
    }
}

void server_accept(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    int addrlen = sizeof(h->address);
    int new_socket = accept(h->socket_fd, (struct sockaddr*)&h->address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        close(h->socket_fd);
        free(h);
    }
    else {
        h->socket_fd = new_socket;
    }
}

int server_receive(void* handle, char* buffer, size_t length) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    return read(h->socket_fd, buffer, length);
}

void server_send(void* handle, const char* message) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    send(h->socket_fd, message, strlen(message), 0);
}

void server_close(void* handle) {
    struct NetworkHandle* h = (struct NetworkHandle*)handle;
    close(h->socket_fd);
    free(h);
}

#endif