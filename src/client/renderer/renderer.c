#include "renderer.h"

#include "client/game_client.h"

uint32_t renderer_create(struct game_client* game) {

	renderer_backend_create(game);

	return 0;
}

uint32_t renderer_destroy(struct game_client* game) {

	renderer_backend_destroy(game);

	return 0;
}

uint32_t renderer_render(struct game_client* game) {

	if (game->application_state.frame_flags & FRAME_FLAG_SHOULD_RENDER == 0) return 0;

	if (game->application_state.frame_flags & FRAME_FLAG_SIZE_CHANGE) {

	}

}

uint32_t renderer_reload_resources(struct game_client* game) {

	renderer_backend_unload_resources(game);

	renderer_backend_load_resources(game);
	
}