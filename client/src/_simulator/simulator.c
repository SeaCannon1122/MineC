#include "minec_client.h"

void simulator_thread_function(struct minec_client* game) {

	game->simulator_state.thread_active_flag = 2;
	while (game->simulator_state.thread_active_flag != 1) sleep_for_ms(1);

	while (game->simulator_state.request_exit_flag == 0) {



		sleep_for_ms(50);

	}

	game->simulator_state.thread_active_flag = 0;

	return;
}

uint32_t simulator_start(struct minec_client* game) {

	game->simulator_state.thread_active_flag = 0;

	game->simulator_state.thread_handle = create_thread(simulator_thread_function, game);

	while (game->simulator_state.thread_active_flag != 2) sleep_for_ms(1);
	game->simulator_state.thread_active_flag = 1;

	return 0;
}


uint32_t simulator_stop(struct minec_client* game) {

	game->simulator_state.request_exit_flag = 1;

	join_thread(game->simulator_state.thread_handle);

	return 0;
}