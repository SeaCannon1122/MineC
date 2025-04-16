#pragma once

#ifndef NETWORKER_H
#define NETWORKER_H

#include <stdint.h>

enum networker_status {
	NETWORKER_STAUS_INACTIVE,
	NETWORKER_STATUS_CONNECTING,
	NETWORKER_STATUS_HANDSHAKE,
	NETWORKER_STATUS_CONNECTED,
	NETWORKER_STATUS_WAITING_ON_CLOSING,
};

enum networker_inactive_reason {
	NETWORKER_REASON_NONE,
	NETWORKER_REASON_CONNECTION_LOST,
	NETWORKER_REASON_COULD_NOT_CONNECT,
	NETWORKER_REASON_RECEIVED_INVALID_PACKETS,
	NETWORKER_REASON_CLOSED_DURING_HANDSHAKE,
	NETWORKER_REASON_CLODED_DURING_CONNECTION,
};

struct networker_state {

	void* thread_handle;

	uint32_t thread_active_flag;
	uint8_t request_exit_flag;

	uint32_t status;
	uint32_t inactive_reason;
	uint32_t handshake_status;

	uint32_t tried_connecting;

	// interaction
	uint8_t request_flag_connect;
	uint8_t ip_nums[4];
	uint16_t port;
	uint8_t ip_reading;
	uint8_t ip_writing;

	uint32_t request_flag_abort_connection; 
	

	void* socket;

};

struct minec_client;

uint32_t networker_start(struct minec_client* game);

uint32_t networker_stop(struct minec_client* game);

#endif // !NETWORKER_H
