#pragma once

#ifndef NETWORKER_H
#define NETWORKER_H

#define MAX_HANDSHAKE_THREADS 8

#include <stdint.h>

struct networker_state {
	void* thread_handle;

	uint32_t thread_active_flag;
	uint8_t request_exit_flag;

	void* socket;

	uint32_t next_handshake_thread_i;
	struct handshake_thread {
		void* thread;
		void* socket;
		uint8_t status;
		uint8_t return_status;
	} handshake_threads[MAX_HANDSHAKE_THREADS];

};

uint32_t networker_start(struct minec_server* game);

uint32_t networker_stop(struct minec_server* game);

#endif // !NETWORKER_H
