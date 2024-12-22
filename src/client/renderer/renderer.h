#ifndef RENDERER_H
#define RENDERER_H

#include "backend/renderer_backend.h"

struct renderer_state {

	struct renderer_backend backend;

	int a;
};

struct game_client;

uint32_t renderer_create(struct game_client* game);

uint32_t renderer_destroy(struct game_client* game);

uint32_t renderer_render(struct game_client* game);

uint32_t renderer_reload_resources(struct game_client* game);

#endif // !RENDERER_H
