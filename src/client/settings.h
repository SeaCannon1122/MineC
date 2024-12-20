#ifndef SETTINGS_H
#define SETIINGS_H

#include <stdint.h>

struct settings_state {

	struct game_settings {

		uint8_t gui_size;

	} game_settings;

	struct game_constants {

		uint32_t server_request_timeout;

	} game_constants;

};

uint32_t settings_initialize(struct game_client* game);

#endif // !SETTINGS_H
