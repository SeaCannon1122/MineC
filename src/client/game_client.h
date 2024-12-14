#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdint.h>
#include "general/platformlib/platform/platform.h"
#include "application/application.h"
#include "graphics/graphics.h"

struct game_client {

	struct application_state application_state;
	struct graphics_state graphics_state;
	

};



int32_t run_game_client(struct game_client* game, uint8_t* resource_path);

#endif // !GAME_CLIENT_H