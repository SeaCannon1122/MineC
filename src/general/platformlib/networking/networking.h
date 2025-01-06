#pragma once

#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdint.h>

enum networking_return_values {
	NETWORKING_SUCCESS                     =        0b0,
	NETWORKING_WRITE_READY                 =        0b1,
	NETWORKING_READ_READY                  =       0b10,
	NETWORKING_ERROR_COULDNT_CREATE_SOCKET =   0b110000,
	NETWORKING_ERROR_CONNECTING            =  0b1010000,
	NETWORKING_ERROR_NOT_READY             =  0b1110000,
	NETWORKING_ERROR_SOCKET_CLOSED         = 0b10010000,
	NETWORKING_ERROR_COULD_NOT_CONNECT     = 0b10110000,
	NETWORKING_ERROR_COULDNT_BIND_TO_PORT  = 0b11010000,
	NETWORKING_ERROR_NO_PEDING_CONNECTIONS = 0b11110000,
	NETWORKING_ERROR                       =    0b10000,
};

uint32_t networking_init();
uint32_t networking_exit();

uint32_t networking_receive_data(void* handle, void* buffer, uint32_t buffer_size, uint32_t* data_size_received);
uint32_t networking_send_data(void* handle, void* buffer, uint32_t buffer_size, uint32_t* data_size_sent);
uint32_t networking_io_status(void* handle);

uint32_t networking_close_connection(void* handle);
uint32_t networking_signal_shutdown(void* handle);

// Server functions
uint32_t networking_server_init(uint16_t port, void** p_server_handle);
uint32_t networking_server_accept(void* server_handle, void** p_client_handle);
uint32_t networking_server_close(void* server_handle);

uint32_t networking_server_get_client_ip(void* client_handle, uint8_t* address_buffer, uint32_t address_buffer_size, uint16_t* port_buffer);

// Client functions
uint32_t networking_client_connect(const uint8_t* ip, uint16_t port, void** p_server_handle);
uint32_t networking_client_connection_status(void* server_handle);

#endif // !NETWORKING_H