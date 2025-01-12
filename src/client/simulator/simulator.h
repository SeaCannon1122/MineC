#pragma once

#ifndef SIMULATOR_H
#define SIMULATOR_H

struct simulator_state {

	void* thread_handle;

	uint32_t thread_active_flag;
	uint32_t simulator_active_bool;

	uint8_t request_exit_flag;
};

struct game_client;

uint32_t simulator_start(struct game_client* game);

uint32_t simulator_stop(struct game_client* game);

#endif // !SIMULATOR_H