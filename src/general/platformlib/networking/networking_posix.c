

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

double networking_get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
}

void networking_init() {}

void networking_exit() {}

bool is_connected(void* socket_handle) {
    int sock = (int)(intptr_t)socket_handle;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // Try to get the peer name (address) to check if the socket is connected
    if (getpeername(sock, (struct sockaddr*)&addr, &addr_len) == 0) {
        return 1; // Socket is connected
    }

    // Check if the error is because the socket is not connected
    if (errno == ENOTCONN) {
        return 0; // Socket is not connected
    }

    // For other errors, consider the socket as disconnected
    return 0;
}

void get_ip_address_and_port(void* client_handle, char* address_buffer, unsigned short* port_buffer) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Retrieve the client's address information from the socket
    if (getpeername((intptr_t)client_handle, (struct sockaddr*)&client_addr, &addr_len) == -1) {
        if (NETWORKING_VERBOSE) perror("Failed to get client address info");
        return;
    }

    // Get client's IP address
    inet_ntop(AF_INET, &(client_addr.sin_addr), address_buffer, INET_ADDRSTRLEN);

    // Get client's port number
    *port_buffer = ntohs(client_addr.sin_port);
}

int available_data_size(void* socket_handle) {
    int available = 0;
    ioctl((intptr_t)socket_handle, FIONREAD, &available);

    return available;
}

int receive_data(void* socket_handle, void* buffer, int size, bool* interrupt, unsigned int timeout_milliseconds) {
    int available = 0;

    bool false_bool = false;
    if (interrupt == NULL) { interrupt = &false_bool; }

    unsigned int start_time = (unsigned int)networking_get_time();

    do {
        // Check how much data is available to read
        if (ioctl((intptr_t)socket_handle, FIONREAD, &available) == -1) {
            if (NETWORKING_VERBOSE) perror("ioctl failed");
            return -1; // Error
        }

        // If there is enough data available, perform the receive operation
        if (available >= size) {
            return recv((intptr_t)socket_handle, buffer, size, 0);
        }

        // Sleep for a short time to avoid busy-waiting
        usleep(10000);  // Sleep for 10 milliseconds
    } while (!*interrupt && (unsigned int)networking_get_time() - start_time <= timeout_milliseconds);

    // Return -2 if interrupted
    return -2;
}

int send_data(void* socket_handle, void* buffer, int size) {
    int sent = 0;

    while (sent < size) {
        int result = send((intptr_t)socket_handle, (const char*)buffer + sent, size - sent, MSG_NOSIGNAL);
        if (result == -1) {
            if (NETWORKING_VERBOSE) perror("send failed");
            return -1; // Error
        }
        sent += result;
    }

    return sent; // Return total bytes sent
}

void close_connection(void* handle) {
    int sock = (int)(intptr_t)handle;
    if (sock >= 0) {
        // Gracefully shutdown the connection
        shutdown(sock, SHUT_RDWR);
        close(sock);
    }
}


void* server_init(int port) {
    int server_socket;
    struct sockaddr_in server_addr;
    struct linger linger_opt;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (NETWORKING_VERBOSE) perror("Could not create socket");
        return NULL;
    }

    // Set SO_LINGER
    linger_opt.l_onoff = 1;   // Enable SO_LINGER
    linger_opt.l_linger = 5; // Set linger timeout to 10 seconds
    if (setsockopt(server_socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0) {
        if (NETWORKING_VERBOSE) perror("Failed to set SO_LINGER");
        close(server_socket);
        return NULL;
    }

    // Make socket non-blocking
    int flags = fcntl(server_socket, F_GETFL, 0); // Get current socket flags
    if (flags == -1) {
        if (NETWORKING_VERBOSE) perror("Could not get socket flags");
        close(server_socket);
        return NULL;
    }

    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) { // Set non-blocking mode
        if (NETWORKING_VERBOSE) perror("Could not set non-blocking mode");
        close(server_socket);
        return NULL;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        if (NETWORKING_VERBOSE) perror("Bind failed");
        close(server_socket);
        return NULL;
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        if (NETWORKING_VERBOSE) perror("Listen failed");
        close(server_socket);
        return NULL;
    }

    return (void*)(intptr_t)server_socket;
}

void* server_accept(void* server_handle) {
    int client_socket = -1; // Initialize with an invalid socket value
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    client_socket = accept((intptr_t)server_handle, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {

        // Handle specific errors
        if (errno == EINTR) {
            // Interrupted system call, retry accept
            return NULL;
        }

        if (NETWORKING_VERBOSE) perror("Accept failed");
        return NULL;
    }

    return (void*)(intptr_t)client_socket;

}


void server_close(void* handle) {
    int server_sock = (int)(intptr_t)handle;
    if (server_sock >= 0) {
        // Here you would need to close all client connections first
        // For simplicity, let's assume you've done that already

        // Gracefully shutdown the server socket
        shutdown(server_sock, SHUT_RDWR);
        close(server_sock);
    }
}

void* client_connect(const char* ip, int port, bool* interrupt, unsigned int timeout_milliseconds) {
    int client_socket;
    struct sockaddr_in server_addr;
    fd_set writefds;
    struct timeval timeout;
    struct linger linger_opt;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    // Set SO_LINGER
    linger_opt.l_onoff = 1;   // Enable SO_LINGER
    linger_opt.l_linger = 5; // Set linger timeout to 10 seconds
    if (setsockopt(client_socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0) {
        perror("Failed to set SO_LINGER");
        close(client_socket);
        return NULL;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
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
        return NULL;
    }

    unsigned int start_time = (unsigned int)networking_get_time();

    // Wait for connection to complete or interrupt
    while (true) {
        if (*interrupt || (unsigned int)networking_get_time() - start_time > timeout_milliseconds) {
            if (NETWORKING_VERBOSE) printf("Client connect interrupted.\n");
            close(client_socket);
            return NULL;
        }

        FD_ZERO(&writefds);
        FD_SET(client_socket, &writefds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // 100 ms

        int activity = select(client_socket + 1, NULL, &writefds, NULL, &timeout);

        if (activity < 0) {
            perror("Select error");
            close(client_socket);
            return NULL;
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
                return NULL;
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