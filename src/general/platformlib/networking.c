#include "networking.h"
#include <stdio.h>
#include <stdbool.h>

/*
initialisation
*/

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#ifdef _WIN32
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


int is_connected(void* socket_handle) {
    char buffer;
    int result;

#ifdef _WIN32
    result = recv((SOCKET)socket_handle, &buffer, 1, MSG_PEEK);
    if (result == 0) {
        // Connection closed
        return 0;
    } else if (result == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        if (error_code == WSAEWOULDBLOCK || error_code == WSAEINPROGRESS) {
            // No data available, but still connected
            return 1;
        } else {
            // Error, connection probably closed
            return 0;
        }
    }
#else
    result = recv(*(int*)&socket_handle, &buffer, 1, MSG_PEEK);
    if (result == 0) {
        // Connection closed
        return 0;
    } else if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available, but still connected
            return 1;
        } else {
            // Error, connection probably closed
            return 0;
        }
    }
#endif

    return 1;  // Socket is still connected
}






/*
server code
*/

#ifdef _WIN32


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

    while (!*interrupt) {
        client_socket = accept((SOCKET)server_handle, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            if (*interrupt) break;
            if (NETWORKING_VERBOSE) printf("Accept failed.\n");
            continue;
        }

        // Get client's IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected from IP: %s, Port: %d\n", client_ip, ntohs(client_addr.sin_port));

        return (void*)client_socket;
    }

    return NULL;
}

int server_send(void* client_handle, const void* buffer, int size) {
    return send((SOCKET)client_handle, buffer, size, 0);
}

int server_receive(void* client_handle, void* buffer, int size) {
    u_long available = 0;

    // Check how much data is available to read
    if (ioctlsocket((SOCKET)client_handle, FIONREAD, &available) == SOCKET_ERROR) {
        if (NETWORKING_VERBOSE) printf("ioctlsocket failed\n");
        return -1; // Error
    }

    // If there is less data available than requested, return 0
    if (available < (u_long)size) {
        return 0; // Not enough data available yet
    }

    // Enough data is available, perform the receive operation
    return recv((SOCKET)client_handle, buffer, size, 0);
}

void server_close_client(void* client_socket) {
    if (closesocket((SOCKET)client_socket) == SOCKET_ERROR) {
        printf("Failed to close client socket: %d\n", WSAGetLastError());
    }
}

void server_close(void* handle) {
    closesocket((SOCKET)handle);
}

#else // Linux implementation


void* server_init(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (NETWORKING_VERBOSE) perror("Could not create socket");
        return NULL;
    }

    // Make socket non-blocking
    int flags = fcntl(server_socket, F_GETFL, 0); // Get current socket flags
    if (flags == -1) {
        if (NETWORKING_VERBOSE) perror("Could not get socket flags");
        return NULL;
    }

    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) { // Set non-blocking mode
        if (NETWORKING_VERBOSE) perror("Could not set non-blocking mode");
        return NULL;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        if (NETWORKING_VERBOSE) perror("Bind failed");
        return NULL;
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        if (NETWORKING_VERBOSE) perror("Listen failed");
        return NULL;
    }

    return (void*)(intptr_t)server_socket;
}

void* server_accept(void* server_handle, bool* interrupt) {
    int client_socket = -1; // Initialize with an invalid socket value
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while (!*interrupt) {
        client_socket = accept((intptr_t)server_handle, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            if (*interrupt) break;

            // Handle specific errors
            if (errno == EINTR) {
                // Interrupted system call, retry accept
                continue;
            }

            if (NETWORKING_VERBOSE) perror("Accept failed");
            continue;
        }

        // Get client's IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected from IP: %s, Port: %d\n", client_ip, ntohs(client_addr.sin_port));

        return (void*)(intptr_t)client_socket;
    }

    return NULL;
}


int server_send(void* client_handle, const void* buffer, int size) {
    return send((intptr_t)client_handle, buffer, size, 0);
}

int server_receive(void* client_handle, void* buffer, int size) {
    int available = 0;

    // Check how much data is available to read
    if (ioctl((int)client_handle, FIONREAD, &available) == -1) {
        if (NETWORKING_VERBOSE) perror("ioctl failed");
        return -1; // Error
    }

    // If there is less data available than requested, return 0
    if (available < size) {
        return 1; // Not enough data available yet
    }

    // Enough data is available, perform the receive operation
    int bytes_count = recv((intptr_t)client_handle, buffer, size, 0);
    if (bytes_count <= 0) return -1;
    return 0;
}

void server_close_client(void* client_socket) {
    if (close((int)client_socket) < 0) {
        perror("Failed to close client socket");
    }
}

void server_close(void* handle) {
    close((intptr_t)handle);
}

#endif

/*
client code
*/

#ifdef _WIN32

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
    u_long available = 0;

    // Check how much data is available to read
    if (ioctlsocket((SOCKET)client_handle, FIONREAD, &available) == SOCKET_ERROR) {
        if (NETWORKING_VERBOSE) printf("ioctlsocket failed\n");
        return -1; // Error
    }

    // If there is less data available than requested, return 0
    if (available < (u_long)size) {
        return 0; // Not enough data available yet
    }

    // Enough data is available, perform the receive operation
    return recv((SOCKET)client_handle, buffer, size, 0);
}

void client_close(void* handle) {
    closesocket((SOCKET)handle);
}

#else // Linux implementation

void* client_connect(const char* server_ip, int server_port, bool* interrupt) {
    int client_socket;
    struct sockaddr_in server_addr;
    fd_set writefds;
    struct timeval timeout;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return NULL;  // Return NULL on failure
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        return NULL;
    }

    // Set socket to non-blocking
    fcntl(client_socket, F_SETFL, O_NONBLOCK);

    // Attempt to connect
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 && errno != EINPROGRESS) {
        perror("Connection failed");
        close(client_socket);
        return NULL;  // Return NULL if connection failed
    }

    // Wait for connection to complete or interrupt
    while (true) {
        if (*interrupt) {
            if (NETWORKING_VERBOSE) printf("Client connect interrupted.\n");
            close(client_socket);
            return NULL;  // Return NULL if interrupted
        }

        FD_ZERO(&writefds);
        FD_SET(client_socket, &writefds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select(client_socket + 1, NULL, &writefds, NULL, &timeout);

        if (activity < 0) {
            perror("Select error");
            close(client_socket);
            return NULL;  // Return NULL on select error
        }
        else if (activity == 0) {
            // Timeout, continue waiting
            continue;
        }
        else if (FD_ISSET(client_socket, &writefds)) {
            // Check if the socket is connected
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(client_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
                perror("Getsockopt error");
                close(client_socket);
                return NULL;  // Return NULL on getsockopt error
            }
            if (error == 0) {
                // Connection successful
                return (void*)(intptr_t)client_socket;
            }
            else {
                // Connection failed
                perror("Connection failed");
                close(client_socket);
                return NULL;
            }
        }
    }
}




int client_send(void* client_handle, const void* buffer, int size) {
    return send((intptr_t)client_handle, buffer, size, 0);
}

int client_receive(void* client_handle, void* buffer, int size) {
    int available = 0;

    // Check how much data is available to read
    if (ioctl((intptr_t)client_handle, FIONREAD, &available) == -1) {
        if (NETWORKING_VERBOSE) perror("ioctl failed");
        return -1; // Error
    }

    // If there is less data available than requested, return 0
    if (available < size) {
        return 0; // Not enough data available yet
    }

    // Enough data is available, perform the receive operation
    return recv((intptr_t)client_handle, buffer, size, 0);
}

void client_close(void* handle) {
    close((intptr_t)handle);
}

#endif
