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

	struct handshake_thread {
		void* thread;
		void* socket;
		uint8_t status;
		uint8_t return_status;
	} handshake_threads[MAX_HANDSHAKE_THREADS];

};

uint32_t networker_start(struct game_server* game);

uint32_t networker_stop(struct game_server* game);

#endif // !NETWORKER_H
