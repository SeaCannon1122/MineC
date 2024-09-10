#pragma once


void networking_init();
void networking_exit();

// Server functions
void* server_init(int port);
void* server_accept(void* server_handle);
int server_send(void* client_handle, const void* data, int size);
int server_receive(void* client_handle, void* buffer, int size);
void server_close(void* handle);

// Client functions
void* client_connect(const char* server_ip, int port);
int client_send(void* client_handle, const void* data, int size);
int client_receive(void* client_handle, void* buffer, int size);
void client_close(void* client_handle);