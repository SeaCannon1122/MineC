#include "game_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "general/platformlib/networking.h"


int32_t run_game_client(struct game_client* game, uint8_t* resource_path) {

	application_create(game);

	while (application_handle_events(game) == 0) {

	}

	application_destroy(game);

	return 0;
}