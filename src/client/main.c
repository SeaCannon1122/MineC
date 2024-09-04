#include "game.h"

void blocks_main() {

	struct game* game = new_game();

	run_game(game, "resources/game_resources.resourcelayout");

	return;
}