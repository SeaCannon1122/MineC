#pragma once

#include <stdbool.h>

#define NETWORKING_VERBOSE 0

void networking_init();
void networking_exit();

bool is_connected(void* socket_handle);
void get_ip_address_and_port(void* client_handle, char* address_buffer, unsigned short* port_buffer);

int receive_data(void* socket_handle, void* buffer, int size, bool* interrupt, unsigned int timeout_milliseconds);
int send_data(void* socket_handle, void* buffer, int size);

void close_connection(void* handle);

// Server functions
void* server_init(int port);
void* server_accept(void* server_handle);
void server_close(void* handle);

// Client functions
void* client_connect(const char* ip, int port, bool* interrupt, unsigned int timeout_milliseconds);