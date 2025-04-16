#pragma once

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

struct settings_state {

	struct server_settings {

		uint8_t accept_new_clients_bool;

	} server_settings;

	struct server_constants {

		uint16_t port;

	} server_constants;

};

struct minec_server;

uint32_t settings_load(struct minec_server* game);

uint32_t settings_save(struct minec_server* game);

#endif // !SETTINGS_H
