#include "game.h"

#include "platform.h"

int main(int argc, char* argv[]) {

	platform_init();

	show_console_window();

	struct game* game = new_game();

	run_game(game, "resources/game_resources.resourcelayout");

	platform_exit();

	return 0;
}