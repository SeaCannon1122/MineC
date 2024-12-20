#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdint.h>
#include "general/platformlib/platform/platform.h"
#include "general/resource_manager.h"
#include "application/application.h"
#include "graphics/graphics.h"
#include "renderer/renderer.h"

struct game_client {

	struct application_state application_state;
	struct graphics_state graphics_state;
	struct resource_manager resource_manager;
	struct renderer renderer;

};



int32_t run_game_client(struct game_client* game, uint8_t* resource_path);

#endif // !GAME_CLIENT_H