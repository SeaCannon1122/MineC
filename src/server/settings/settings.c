#include "server/game_server.h"

uint32_t settings_load(struct game_server* game) {

	game->settings_state.server_constants.port = 12345;

	return 0;
}

uint32_t settings_save(struct game_server* game) {

	return 0;
}