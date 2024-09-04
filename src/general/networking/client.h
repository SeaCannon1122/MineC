#pragma once 

#include <stddef.h>

void* client_connect(const char* ip, int port);
int client_receive(void* handle, char* buffer, size_t length);
void client_send(void* handle, const char* message);
void client_close(void* handle);