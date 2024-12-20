#include "resources.h"

#include <stdio.h>

#include "client/game_client.h"

uint32_t resources_create(struct game_client* game, uint32_t* resource_path) {
	resource_manager_new(&game->resource_state.resource_manager, resource_path);

	sprintf(game->resource_state.resource_manager_root_path, resource_path);



	for (uint32_t i = 0; i < TEXTURE_ATLAS_LENGTH; i++)
		game->resource_state.texture_atlas[i] = resource_manager_get_image_index(&game->resource_state.resource_manager, resource_images_names[i]);

	resources_switch_use_vulkan_device(game);

	return 0;
}

uint32_t resources_switch_use_vulkan_device(struct game_client* game) {
	resource_manager_use_vulkan_device(&game->resource_state.resource_manager, game->graphics_state.device, game->graphics_state.gpu, game->graphics_state.queue, game->graphics_state.queue_index);
	


	return 0;
}

uint32_t resources_switch_drop_vulkan_device(struct game_client* game) {
	resource_manager_drop_vulkan_device(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_destroy(struct game_client* game) {
	resource_manager_destroy(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_reload(struct game_client* game) {

	resources_destroy(game);
	resources_create(game, game->resource_state.resource_manager_root_path);

	return 0;
}