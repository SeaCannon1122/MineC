#include "networking.h"
#include <stdio.h>




/*
initialisation
*/


#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>

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

void networking_init() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

void networking_exit() {
    WSACleanup();
}

void* server_init(int port) {
    SOCKET server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket.\n");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        return NULL;
    }

    listen(server_socket, 3);
    return (void*)server_socket;
}

void* server_accept(void* server_handle) {
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    int addr_len = sizeof(struct sockaddr_in);

    client_socket = accept((SOCKET)server_handle, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed.\n");
        return NULL;
    }

    return (void*)client_socket;
}

int server_send(void* client_handle, const void* buffer) {
    return send((SOCKET)client_handle, buffer, strlen((const char*)buffer), 0);
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

void networking_init() {}

void networking_exit() {}

void* server_init(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Could not create socket");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return NULL;
    }

    listen(server_socket, 3);
    return (void*)(intptr_t)server_socket;
}

void* server_accept(void* server_handle) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    client_socket = accept((intptr_t)server_handle, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {
        perror("Accept failed");
        return NULL;
    }

    return (void*)(intptr_t)client_socket;
}

int server_send(void* client_handle, const void* buffer) {
    return send((intptr_t)client_handle, buffer, strlen((const char*)buffer), 0);
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

void* client_connect(const char* ip, int port) {
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket.\n");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        return NULL;
    }

    return (void*)client_socket;
}

int client_send(void* client_handle, const void* buffer) {
    return send((SOCKET)client_handle, buffer, strlen((const char*)buffer), 0);
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

void* client_connect(const char* ip, int port) {
    int client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Could not create socket");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return NULL;
    }

    return (void*)(intptr_t)client_socket;
}

int client_send(void* client_handle, const void* buffer) {
    return send((intptr_t)client_handle, buffer, strlen((const char*)buffer), 0);
}

int client_receive(void* client_handle, void* buffer, int size) {
    return recv((intptr_t)client_handle, buffer, size, 0);
}

void client_close(void* handle) {
    close((intptr_t)handle);
}

#endif
