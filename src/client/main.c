﻿#include "game.h"

void Entry() {

	printf("hi");

	struct game* game = new_game();

	run_game(game, "resources/game_resources.resourcelayout");

	return;
}