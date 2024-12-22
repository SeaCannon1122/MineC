#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdint.h>
#include "general/platformlib/platform/platform.h"
#include "resources/resources.h"
#include "resources/settings.h"
#include "application/application.h"
#include "renderer/renderer.h"
#include "simulator/simulator.h"
#include "simulator/networker.h"
#include "gui/gui.h"

struct game_client {

	struct resource_state resource_state;
	struct settings_state settings_state;
	struct application_state application_state;
	struct renderer_state renderer_state;
	struct simulator_state simulator_state;
	struct networker_state networker_state;
	struct gui_state gui_state;

};


uint32_t game_client_run(struct game_client* game, uint8_t* resource_path);

#endif // !GAME_CLIENT_H