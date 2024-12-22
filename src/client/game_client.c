#include "game_client.h"

uint32_t game_client_run(struct game_client* game, uint8_t* resource_path) {

	resources_create(game, resource_path);

	application_create(game);

	renderer_create(game);

	while (application_handle_events(game) == 0) {

		renderer_render(game);

	}

	renderer_destroy(game);

	application_destroy(game);

	resources_destroy(game);

	return 0;
}