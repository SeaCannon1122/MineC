#include "game_server_body/game_server.h"

#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"

int main(int argc, char* argv[]) {

	platform_init();
	networking_init();

	struct game_server game;
	new_game_server(&game, "resources/server/resourcelayout.keyvalue");

	run_game_server(&game);

	delete_game_server(&game);

	networking_exit();
	platform_exit();

	return 0;
}