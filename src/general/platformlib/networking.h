#pragma once

#include <stdbool.h>

#define NETWORKING_VERBOSE 0

void networking_init();
void networking_exit();

int is_connected(void* socket_handle);

// Server functions
void* server_init(int port);
void* server_accept(void* server_handle, bool* interrupt);
int server_send(void* client_handle, const void* data, int size);
int server_receive(void* client_handle, void* buffer, int size);
void server_close_client(void* client_handle);
void server_close(void* handle);

// Client functions
void* client_connect(const char* server_ip, int port, bool* interrupt);
int client_send(void* client_handle, const void* data, int size);
int client_receive(void* client_handle, void* buffer, int size);
void client_close(void* client_handle);