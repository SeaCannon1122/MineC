#pragma once

#include <stddef.h>

void* server_init(int port);
void server_listen(void* handle);
void server_accept(void* handle);
int server_receive(void* handle, char* buffer, size_t length);
void server_send(void* handle, const char* message);
void server_close(void* handle);