#include "server/minec_server.h"

uint32_t settings_load(struct minec_server* game) {

	game->settings_state.server_constants.port = 12345;

	return 0;
}

uint32_t settings_save(struct minec_server* game) {

	return 0;
}