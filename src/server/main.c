#include "game_server.h"

#include "general/platformlib/networking.h"
#include "general/platformlib/platform/platform.h"

#include <stdio.h>

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	show_console_window();

	struct game_server game;
	uint32_t game_return_status = game_server_run(&game, "../../../resources/server/root.resourcelayout.yaml");

	if (game_return_status != 0) {
		printf("Press RETURN to exit ...");
		getchar();
	}

	networking_exit();
	platform_exit();

	return 0;
}