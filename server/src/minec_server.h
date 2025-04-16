#pragma once

#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

#include "general/resource_manager.h"
#include "general/platformlib/platform/platform.h"
#include "general/platformlib/networking/networking.h"

#include "resources/resources.h"
#include "networker/networker.h"
#include "settings/settings.h"

#include "game/game.h"

struct minec_server {
	
	struct resource_state resource_state;
	struct networker_state networker_state;
	struct settings_state settings_state;


};

uint32_t game_server_run(struct minec_server* game, uint8_t* resource_path);

#endif // !GAME_SERVER_H





