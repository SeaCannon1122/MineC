#include "gui.h"

#include "general/rendering/gui/pixel_char.h"

#include "client/game_client.h"

uint32_t gui_create(struct game_client* game) {

	

	gui_use_device(game);

	return 0;
}

uint32_t gui_use_device(struct game_client* game) {

	pixel_char_renderer_new(&game->gui_state.pixel_char_renderer, &game->graphics_state.rmm, game->graphics_state.device, game->graphics_state.window_render_pass);

	return 0;
}

uint32_t gui_drop_device(struct game_client* game) {

	pixel_char_renderer_destroy(&game->gui_state.pixel_char_renderer);

	return 0;
}

uint32_t gui_destroy(struct game_client* game) {

	return 0;
}