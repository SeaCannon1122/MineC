#pragma once

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

struct settings_state {

	struct game_settings {

		uint32_t gui_scale;

	} game_settings;

	struct game_constants {

		uint32_t server_request_timeout;

	} game_constants;

};

struct minec_client;

uint32_t settings_load(struct minec_client* game);

uint32_t settings_save(struct minec_client* game);

#endif // !SETTINGS_H
