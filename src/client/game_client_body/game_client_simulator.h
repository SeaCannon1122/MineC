#pragma once

#include "game_client.h"

struct game_simulation_state_chunk {
	struct {
		int x;
		int y;
		int z;
	} coordinates;
	struct {
		short block_id
	} blocks[16 * 16 * 16]};
};

struct game_simulation_state {

	
};

void game_client_simulator_thread_function(struct game_client* game);