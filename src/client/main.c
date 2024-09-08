#include "game_body/game_client.h"
#include "general/parallel_computing.h"

#include "platform.h"

int main(int argc, char* argv[]) {

	platform_init();
	parallel_computing_init();



	struct game_client* game = new_game_client("../../../resources/client/resourcelayout.keyvalue");

	run_game_client(game);



	parallel_computing_exit();
	platform_exit();

	return 0;
}