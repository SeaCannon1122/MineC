#pragma once

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <general/platformlib/platform/platform.h>
#include <general/platformlib/networking/networking.h>

#include "resources/resources.h"
#include "application/application.h"
#include "renderer/renderer.h"
#include "gui/gui_menus.h"
#include "settings/settings.h"
#include "simulator/simulator.h"
#include "networker/networker.h"

#include "game/game.h"

static uint32_t game_strlen(uint32_t* str) {
	uint32_t len = 0;
	for (; str[len] != 0; len++);
	return len;
}

struct game_client {

	struct settings_state settings_state;

	struct resource_state resource_state;
	struct application_state application_state;
	struct renderer_state renderer_state;
	struct gui_menus_state gui_menus_state;
	struct simulator_state simulator_state;
	struct networker_state networker_state;
	void* simulator_thread_handle;
	void* networker_thread_handle;
};


uint32_t game_client_run(struct game_client* game, uint8_t* resource_path);

#endif // !GAME_CLIENT_H