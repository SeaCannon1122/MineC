#include "game_client_renderer.h"

#include "general/platformlib/platform.h"

void client_render_world(struct game_client* game) {
	for (int i = 0; i < game->render_state.height * game->render_state.width; i++) game->render_state.pixels[i] = 0xff6c87bf;
}