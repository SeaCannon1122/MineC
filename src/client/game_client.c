#include "game_client.h"

uint32_t run_game_client(struct game_client* game, uint8_t* resource_path) {

	application_create(game);

	if (resources_create(game, resource_path) != 0) {
		application_destroy(game);
		return 1;
	}

	while (application_handle_events(game) == 0) {



	}
	

	resources_destroy(game);

	application_destroy(game);

	return 0;
}