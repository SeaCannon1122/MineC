#include "game_client.h"
#include "general/platformlib/networking.h"
#include "general/platformlib/platform/platform.h"
#include <stdio.h>

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	show_console_window();

	struct game_client game;
	uint32_t game_return_status = game_client_run(&game, "../../../resources/client/root.resourcelayout.yaml");

	if (game_return_status != 0) {
		printf("Press any key to exit ...");
		getchar();
	}

	networking_exit();
	platform_exit();

	return 0;
}