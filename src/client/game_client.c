#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "general/platformlib/networking.h"


int32_t run_game_client(struct game_client* game, uint8_t* resource_path) {

	application_create(game);

	resource_manager_new(&game->resource_manager, resource_path);
	resource_manager_use_vulkan_device(&game->resource_manager, game->graphics_state.device, game->graphics_state.gpu, game->graphics_state.queue, game->graphics_state.queue_index);

	while (application_handle_events(game) == 0) {

	}

	resource_manager_destroy(&game->resource_manager);

	application_destroy(game);

	return 0;
}