#include "game_server_body/game_server.h"

#include "general/platformlib/parallel_computing.h"
#include "general/platformlib/networking.h"
#include "general/platformlib/platform.h"

int main(int argc, char* argv[]) {

	networking_init();
	parallel_computing_init();


	struct game_server* game = new_game_server("../../../resources/server/resourcelayout.keyvalue");

	run_game_server(game);

	delete_game_server(game);


	parallel_computing_exit();
	networking_exit();

	return 0;
}