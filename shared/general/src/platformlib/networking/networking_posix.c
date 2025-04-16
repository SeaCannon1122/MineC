#include "networking.h"
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

uint32_t networking_init() {
    return 0;
}

uint32_t networking_exit() {
    return 0;
}

uint32_t networking_receive_data(void* handle, void* buffer, uint32_t buffer_size, uint32_t* data_size_received) {

    int recv_return_value = recv((intptr_t)handle, buffer, buffer_size, 0);
    if (recv_return_value == 0) return NETWORKING_ERROR_SOCKET_CLOSED;

    if (recv_return_value == SOCKET_ERROR) {
        int error_code = errno;
        if (error_code == EWOULDBLOCK) return NETWORKING_ERROR_NOT_READY;
        if (error_code == ECONNRESET || error_code == ECONNABORTED) return NETWORKING_ERROR_SOCKET_CLOSED;

        printf("ERRNO: recv %d\n", error_code);
        return NETWORKING_ERROR;
    }

    *data_size_received = recv_return_value;
    return NETWORKING_SUCCESS;
}

uint32_t networking_send_data(void* handle, void* buffer, uint32_t buffer_size, uint32_t* data_size_sent) {

    int send_return_value = send((intptr_t)handle, (char*)buffer, buffer_size, 0);
    if (send_return_value == 0) return NETWORKING_ERROR_SOCKET_CLOSED;

    if (send_return_value == SOCKET_ERROR) {
        int error_code = errno;
        if (error_code == EWOULDBLOCK) return NETWORKING_ERROR_NOT_READY;
        if (error_code == ECONNRESET || error_code == ECONNABORTED) return NETWORKING_ERROR_SOCKET_CLOSED;

        printf("ERRNO: send %d\n", error_code);
        return NETWORKING_ERROR;
    }
    *data_size_sent = send_return_value;
    return NETWORKING_SUCCESS;
}

uint32_t networking_io_status(void* handle) {
    fd_set readfds, writefds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET((intptr_t)handle, &readfds);
    FD_SET((intptr_t)handle, &writefds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int select_return_value = select(0, &readfds, &writefds, NULL, &timeout);

    if (select_return_value == SOCKET_ERROR) {
        int error_code = errno;
        if (error_code == ENOTCONN || error_code == ECONNABORTED) return NETWORKING_ERROR_SOCKET_CLOSED;

        printf("ERRNO: select %d\n", error_code);
        return NETWORKING_ERROR;
    }

    uint32_t return_value = 0;
    if (FD_ISSET((intptr_t)handle, &readfds)) return_value |= NETWORKING_READ_READY;
    if (FD_ISSET((intptr_t)handle, &writefds)) return_value |= NETWORKING_WRITE_READY;

    return return_value;
}

uint32_t networking_signal_shutdown(void* handle) {

    int shutdown_return_value = shutdown((intptr_t)handle, SHUT_WR);

    if (shutdown_return_value == SOCKET_ERROR) {
        printf("ERRNO: shutdown %d\n", errno);
        return NETWORKING_ERROR;
    }
    return NETWORKING_SUCCESS;
}

uint32_t networking_close_connection(void* handle) {

    if (close((intptr_t)handle) == SOCKET_ERROR) {
        printf("ERRNO: close %d\n", errno);
        return NETWORKING_ERROR;
    }
    return NETWORKING_SUCCESS;
}

uint32_t networking_server_init(uint16_t port, void** p_server_handle) {

    int server_socket;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) return NETWORKING_ERROR_COULDNT_CREATE_SOCKET;

    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1) {
        printf("ERRNO %d\n", errno);
        close(server_socket);
        return NETWORKING_ERROR;
    }

    flags |= O_NONBLOCK;
    if (fcntl(server_socket, F_SETFL, flags) == -1) {
        printf("ERRNO %d\n", errno);
        close(server_socket);
        return NETWORKING_ERROR;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) return NETWORKING_ERROR_COULDNT_BIND_TO_PORT;

    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("ERRNO: listen %d\n", errno);
        close(server_socket);
        return NETWORKING_ERROR;
    }

    *p_server_handle = server_socket;
    return NETWORKING_SUCCESS;
}

uint32_t networking_server_accept(void* server_handle, void** p_client_handle) {
    int client_socket;
    struct sockaddr_in client_addr;
    int addr_len = sizeof(struct sockaddr_in);

    client_socket = accept((intptr_t)server_handle, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET) {
        int error_code = errno;
        if (error_code == EWOULDBLOCK) return NETWORKING_ERROR_NO_PEDING_CONNECTIONS;

        printf("accept failed. Error: %d\n", error_code);
        return NETWORKING_ERROR;
    }

    *p_client_handle = client_socket;
    return NETWORKING_SUCCESS;
}

uint32_t networking_server_close(void* server_handle) {
    if (shutdown((intptr_t)server_handle, SHUT_RDWR) == SOCKET_ERROR) {
        printf("shutdown failed. Error: %d\n", errno);
        return NETWORKING_ERROR;
    }
    if (close((intptr_t)server_handle) == SOCKET_ERROR) {
        printf("close failed. Error: %d\n", errno);
        return NETWORKING_ERROR;
    }
    return NETWORKING_SUCCESS;
}

uint32_t networking_server_get_client_ip(void* client_handle, uint8_t* address_buffer, uint32_t address_buffer_size, uint16_t* port_buffer) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (getpeername((intptr_t)client_handle, (struct sockaddr*)&client_addr, &addr_len) == SOCKET_ERROR) {
        printf("getpeername failed. Error: %d\n", errno);
        return NETWORKING_ERROR;
    }

    if (inet_ntop(AF_INET, &(client_addr.sin_addr), address_buffer, address_buffer_size) == NULL) {
        printf("inet_ntop failed. Error: %d\n", errno);
        return NETWORKING_ERROR;
    }

    *port_buffer = ntohs(client_addr.sin_port);
    return NETWORKING_SUCCESS;
}


uint32_t networking_client_connect(const uint8_t* ip, uint16_t port, void** p_server_handle) {
    int client_socket;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) return NETWORKING_ERROR_COULDNT_CREATE_SOCKET;

    int flags = fcntl(client_socket, F_GETFL, 0);
    if (flags == -1) {
        printf("ERRNO %d\n", errno);
        close(client_socket);
        return NETWORKING_ERROR;
    }

    flags |= O_NONBLOCK;
    if (fcntl(client_socket, F_SETFL, flags) == -1) {
        printf("ERRNO %d\n", errno);
        close(client_socket);
        return NETWORKING_ERROR;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        int error_code = errno;
        if (error_code != EWOULDBLOCK && error_code != EINPROGRESS) {
            printf("ERRNO %d\n", error_code);
            close(client_socket);
            return NETWORKING_ERROR;
        }
    }

    *p_server_handle = client_socket;
    return NETWORKING_SUCCESS;
}

uint32_t networking_client_connection_status(void* server_handle) {

    fd_set write_fds;

    FD_ZERO(&write_fds);
    FD_SET((intptr_t)server_handle, &write_fds);
    struct timeval timeout = { 0, 0 };

    int result = select((size_t)server_handle + 1, NULL, &write_fds, NULL, &timeout);
    if (result == SOCKET_ERROR) return NETWORKING_ERROR;
    if (result == 0) {

        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(server_handle, SOL_SOCKET, SO_ERROR, &error, &len) != 0) {
            perror("getsockopt failed");
            return NETWORKING_ERROR;
        }
        if (error != 0 && error != EINPROGRESS) return NETWORKING_ERROR_COULD_NOT_CONNECT;
        return NETWORKING_ERROR_CONNECTING;
    }

    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(server_handle, SOL_SOCKET, SO_ERROR, &error, &len) != 0) {
        perror("getsockopt failed");
        return NETWORKING_ERROR;
    }

    if (error == 0) return NETWORKING_SUCCESS;
    if (error == EINPROGRESS) return NETWORKING_ERROR_CONNECTING;
    else return NETWORKING_ERROR_COULD_NOT_CONNECT;

}