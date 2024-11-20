#pragma once

struct game_renderer {
	int a;
};

struct game_client;

void renderer_init(struct game_client* game);

void client_renderer_adjust_size(struct game_client* game);

void client_render_world(struct game_client* game);

void renderer_exit(struct game_client* game);