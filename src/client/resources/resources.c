#include "resources.h"

#include <stdio.h>

#include "client/game_client.h"

uint32_t resources_create(struct game_client* game, uint8_t* resource_path) {
	resource_manager_new(&game->resource_state.resource_manager, resource_path);

	sprintf(game->resource_state.resource_manager_root_path, resource_path);

	for (uint32_t i = 0; i < RESOURCES_IMAGES_COUNT; i++) {
		if (resource_manager_get_image(&game->resource_state.resource_manager, resources_image_tokens[i], &game->resource_state.image_atlas[i])) {

			printf("[GAME RESOURCES] Couldn't find image matching token '%s'\n", resources_image_tokens[i]);
			game->resource_state.image_atlas[i].data = default_image;
			game->resource_state.image_atlas[i].width = default_image_width;
			game->resource_state.image_atlas[i].height = default_image_height;

		}
	}

	game->resource_state.error_pixelfont = NULL;

	for (uint32_t i = 0; i < RESOURCES_PIXELFONTS_COUNT; i++) {
		if (resource_manager_get_pixelfont(&game->resource_state.resource_manager, resources_pixelfont_tokens[i], &game->resource_state.pixelfont_atlas[i])) {

			printf("[GAME RESOURCES] Couldn't find pixelfont matching token '%s'\n", resources_pixelfont_tokens[i]);

			if (game->resource_state.error_pixelfont == NULL) game->resource_state.error_pixelfont = calloc(1, sizeof(struct pixel_font));

			game->resource_state.pixelfont_atlas[i] = game->resource_state.error_pixelfont;

		}
	}

	return 0;
}


uint32_t resources_destroy(struct game_client* game) {

	if (game->resource_state.error_pixelfont != NULL) free(game->resource_state.error_pixelfont);
	
	resource_manager_destroy(&game->resource_state.resource_manager);

	return 0;
}

uint32_t resources_reload(struct game_client* game) {

	resources_destroy(game);
	resources_create(game, game->resource_state.resource_manager_root_path);

	return 0;
}