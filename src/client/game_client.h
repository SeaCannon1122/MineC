#pragma once

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdint.h>
#include "resources/resources.h"
#include "application/application.h"
#include "renderer/renderer.h"
#include "gui/gui_menus.h"
#include "simulator/simulator.h"
#include "simulator/networker.h"

struct game_client {

	struct resource_state resource_state;
	struct application_state application_state;
	struct renderer_state renderer_state;
	struct gui_menus_state gui_menus_state;
	struct simulator_state simulator_state;
	struct networker_state networker_state;
	void* simulator_thread_handle;
};


uint32_t game_client_run(struct game_client* game, uint8_t* resource_path);

#endif // !GAME_CLIENT_H