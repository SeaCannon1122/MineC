#include "client/game_client.h"

uint32_t settings_load(struct game_client* game) {

	game->settings_state.game_settings.gui_scale = 2;

	return 0;
}

uint32_t settings_save(struct game_client* game) {

	return 0;
} 