#include "game.h"

int main() {

	struct game* game = new_game();

	run_game(game, "resources/game_resources.resourcelayout");

	return 0;
}