#pragma once

#if defined(_WIN32)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

struct NetworkHandle {
    SOCKET socket_fd;
    struct sockaddr_in address;
};

#elif defined(__linux__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

struct NetworkHandle {
    int socket_fd;
    struct sockaddr_in address;
};

#endif