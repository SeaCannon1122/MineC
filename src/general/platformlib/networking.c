#include "networking.h"
#include <stdio.h>
#include <stdbool.h>

/*
initialisation
*/

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

void networking_init() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

void networking_exit() {
    WSACleanup();
}

#else

void networking_init() {}

void networking_exit() {}

#endif

/*
server code
*/

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

void* server_init(int port) {
    SOCKET server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        if (NETWORKING_VERBOSE) printf("Could not create socket.\n");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        if (NETWORKING_VERBOSE) printf("Bind failed.\n");
        return NULL;
    }

    listen(server_socket, 3);
    return (void*)server_socket;
}

void* server_accept(void* server_handle, bool* interrupt) {
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    int addr_len = sizeof(struct sockaddr_in);

    fd_set readfds;
    struct timeval timeout;

    while (true) {
        if (*interrupt) {
            if (NETWORKING_VERBOSE) printf("Server accept interrupted.\n");
            return NULL;
        }

        FD_ZERO(&readfds);
        FD_SET((SOCKET)server_handle, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select(0, &readfds, NULL, NULL, &timeout);

        if (activity > 0 && FD_ISSET((SOCKET)server_handle, &readfds)) {
            client_socket = accept((SOCKET)server_handle, (struct sockaddr*)&client_addr, &addr_len);
            if (client_socket == INVALID_SOCKET) {
                if (NETWORKING_VERBOSE) printf("Accept failed.\n");
                return NULL;
            }
            return (void*)client_socket;
        }
    }
}

int server_send(void* client_handle, const void* buffer, int size) {
    return send((SOCKET)client_handle, buffer, size, 0);
}

int server_receive(void* client_handle, void* buffer, int size) {
    return recv((SOCKET)client_handle, buffer, size, 0);
}

void server_close(void* handle) {
    closesocket((SOCKET)handle);
}

#else // Linux implementation
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void* server_init(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (NETWORKING_VERBOSE) perror("Could not create socket");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        if (NETWORKING_VERBOSE) perror("Bind failed");
        return NULL;
    }

    listen(server_socket, 3);
    return (void*)(intptr_t)server_socket;
}

void* server_accept(void* server_handle, bool* interrupt) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    fd_set readfds;
    struct timeval timeout;

    while (true) {
        if (*interrupt) {
            if (NETWORKING_VERBOSE) printf("Server accept interrupted.\n");
            return NULL;
        }

        FD_ZERO(&readfds);
        FD_SET((intptr_t)server_handle, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select((intptr_t)server_handle + 1, &readfds, NULL, NULL, &timeout);

        if (activity > 0 && FD_ISSET((intptr_t)server_handle, &readfds)) {
            client_socket = accept((intptr_t)server_handle, (struct sockaddr*)&client_addr, &addr_len);
            if (client_socket < 0) {
                if (NETWORKING_VERBOSE) perror("Accept failed");
                return NULL;
            }
            return (void*)(intptr_t)client_socket;
        }
    }
}

int server_send(void* client_handle, const void* buffer, int size) {
    return send((intptr_t)client_handle, buffer, size, 0);
}

int server_receive(void* client_handle, void* buffer, int size) {
    return recv((intptr_t)client_handle, buffer, size, 0);
}

void server_close(void* handle) {
    close((intptr_t)handle);
}

#endif

/*
client code
*/

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

void* client_connect(const char* ip, int port, bool* interrupt) {
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        if (NETWORKING_VERBOSE) printf("Could not create socket.\n");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    u_long mode = 1;
    ioctlsocket(client_socket, FIONBIO, &mode);  // Set non-blocking

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
        if (NETWORKING_VERBOSE) printf("Connection failed.\n");
        return NULL;
    }

    fd_set writefds;
    struct timeval timeout;

    while (true) {
        if (*interrupt) {
            if (NETWORKING_VERBOSE) printf("Client connect interrupted.\n");
            closesocket(client_socket);
            return NULL;
        }

        FD_ZERO(&writefds);
        FD_SET(client_socket, &writefds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select(0, NULL, &writefds, NULL, &timeout);

        if (activity > 0 && FD_ISSET(client_socket, &writefds)) {
            // Connection successful
            return (void*)client_socket;
        }
    }
}

int client_send(void* client_handle, const void* buffer, int size) {
    return send((SOCKET)client_handle, buffer, size, 0);
}

int client_receive(void* client_handle, void* buffer, int size) {
    return recv((SOCKET)client_handle, buffer, size, 0);
}

void client_close(void* handle) {
    closesocket((SOCKET)handle);
}

#else // Linux implementation
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void* client_connect(const char* ip, int port, bool* interrupt) {
    int client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (NETWORKING_VERBOSE) perror("Could not create socket");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    fcntl(client_socket, F_SETFL, O_NONBLOCK);  // Set non-blocking

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 && errno != EINPROGRESS) {
        if (NETWORKING_VERBOSE) perror("Connection failed");
        return NULL;
    }

    fd_set writefds;
    struct timeval timeout;

    while (true) {
        if (*interrupt) {
            if (NETWORKING_VERBOSE) printf("Client connect interrupted.\n");
            close(client_socket);
            return NULL;
        }

        FD_ZERO(&writefds);
        FD_SET(client_socket, &writefds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select(client_socket + 1, NULL, &writefds, NULL, &timeout);

        if (activity > 0 && FD_ISSET(client_socket, &writefds)) {
            // Connection successful
            return (void*)(intptr_t)client_socket;
        }
    }
}

int client_send(void* client_handle, const void* buffer, int size) {
    return send((intptr_t)client_handle, buffer, size, 0);
}

int client_receive(void* client_handle, void* buffer, int size) {
    return recv((intptr_t)client_handle, buffer, size, 0);
}

void client_close(void* handle) {
    close((intptr_t)handle);
}

#endif
