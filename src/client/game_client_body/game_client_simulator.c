#include "game_client_simulator.h"

#include "general/platformlib/platform.h"

void game_client_simulator_thread_function(struct game_client* game) {

	while (game->running) {
		sleep_for_ms(500);
	}

}