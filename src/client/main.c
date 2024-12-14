#include "game_client.h"
#include "general/platformlib/networking.h"
#include "general/platformlib/platform/platform.h"

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	show_console_window();

	struct game_client game;
	uint32_t game_return_status = run_game_client(&game, "../../../resources/client/resourcelayout.keyvalue");

	networking_exit();
	platform_exit();

	return 0;
}