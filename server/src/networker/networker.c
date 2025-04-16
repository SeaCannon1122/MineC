#include "server/minec_server.h"

void handshake_thread_function(struct minec_server* game) {

	uint32_t thread_i = game->networker_state.next_handshake_thread_i;

	game->networker_state.handshake_threads[thread_i].status = 1;


	game->networker_state.handshake_threads[thread_i].return_status = 0;

	game->networker_state.handshake_threads[thread_i].status = 99;

	return;
}

void networker_thread_function(struct minec_server* game) {

	game->networker_state.thread_active_flag = 2;
	while (game->networker_state.thread_active_flag != 1) sleep_for_ms(1);

	while (game->networker_state.request_exit_flag == 0) {

		if (game->settings_state.server_settings.accept_new_clients_bool) {

			uint32_t threads_i = 0;
			for (; threads_i < MAX_HANDSHAKE_THREADS; threads_i++) if (game->networker_state.handshake_threads[threads_i].status == 0) break;
			if (threads_i == MAX_HANDSHAKE_THREADS) {
				sleep_for_ms(20);
				continue;
			}

			void* client_handle;
			uint32_t accept_return_value = networking_server_accept(game->networker_state.socket, &client_handle);

			if (accept_return_value != NETWORKING_SUCCESS) sleep_for_ms(20);
			else {

				game->networker_state.handshake_threads[threads_i].socket = client_handle;
				game->networker_state.handshake_threads[threads_i].status = 2;

				create_thread(handshake_thread_function, game);
				while (game->networker_state.handshake_threads[threads_i].status != 1) sleep_for_ms(1);
				


			}


		}

	}
	

	game->networker_state.thread_active_flag = 0;

	return;
}

uint32_t networker_start(struct minec_server* game) {

	uint32_t server_init_return_value = networking_server_init(game->settings_state.server_constants.port, &game->networker_state.socket);

	if (server_init_return_value == NETWORKING_ERROR_COULDNT_BIND_TO_PORT) {

		printf("Couldn't bind server to port %d\n", game->settings_state.server_constants.port);
		return 1;
	}
	else if (server_init_return_value == NETWORKING_ERROR_COULDNT_CREATE_SOCKET) {
		printf("Couldn't create server socket\n");
		return 2;
	}

	game->networker_state.request_exit_flag = 0;
	game->networker_state.thread_active_flag = 0;

	for (uint32_t i = 0; i < MAX_HANDSHAKE_THREADS; i++) game->networker_state.handshake_threads[i].status = 0;

	game->networker_state.thread_handle = create_thread(networker_thread_function, game);

	while (game->networker_state.thread_active_flag != 2) sleep_for_ms(1);
	game->networker_state.thread_active_flag = 1;

	return 0;
}


uint32_t networker_stop(struct minec_server* game) {

	game->networker_state.request_exit_flag = 1;

	join_thread(game->networker_state.thread_handle);

	return 0;
}