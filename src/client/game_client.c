#include "game_client.h"

uint32_t game_client_run(struct game_client* game, uint8_t* resource_path) {

	application_create(game);

	resources_create(game, resource_path);

	renderer_create(game);

	//renderer_backend_use_gpu(game, 2);

	while (application_handle_events(game) == 0) {

		renderer_render(game);

		sleep_for_ms(20);

	}

	renderer_destroy(game);

	resources_destroy(game);

	application_destroy(game);

	return 0;
}