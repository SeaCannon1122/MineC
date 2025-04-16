#include "server/minec_server.h"


uint32_t resources_create(struct minec_server* game, uint8_t* resource_path) {
	resource_manager_new(&game->resource_state.resource_manager, resource_path);

	sprintf(game->resource_state.resource_manager_root_path, resource_path);

	for (uint32_t i = 0; i < RESOURCES_KVM_COUNT; i++) {

		if (resource_manager_get_key_value_map(&game->resource_state.resource_manager, resources_key_value_map_tokens[i], &game->resource_state.key_value_map_atlas[i])) {

			printf("[GAME RESOURCES] Couldn't find KeyValueMap matching token '%s'\n", resources_key_value_map_tokens[i]);

			game->resource_state.key_value_map_atlas[i] = key_value_new(10, 100);
		}

	}

	return 0;
}


uint32_t resources_destroy(struct minec_server* game) {

	resource_manager_destroy(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_reload(struct minec_server* game) {

	resources_destroy(game);
	resources_create(game, game->resource_state.resource_manager_root_path);

	return 0;
}