#pragma once

struct game {
	int a;
};

struct game* new_game();

void run_game(struct game* game, char* resource_path);