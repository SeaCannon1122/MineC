#include "simulator.h"

#include "client/game_client.h"

#include "general/platformlib/platform/platform.h"

uint32_t simulator_thread_function(struct game_client* game) {

	return 0;
}

uint32_t simulator_start(struct game_client* game) {

	game->simulator_thread_handle = create_thread(simulator_thread_function, game);

	return 0;
}


uint32_t simulator_stop(struct game_client* game) {

	join_thread(game->simulator_thread_handle);

	return 0;
}