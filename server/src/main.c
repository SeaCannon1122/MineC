#include "minec_server.h"

int main(int argc, char* argv[]) {

	networking_init();

	struct minec_server game;
	uint32_t game_return_status = game_server_run(&game, "../../../resources/server/root.resourcelayout.yaml");

	networking_exit();

	return 0;
}