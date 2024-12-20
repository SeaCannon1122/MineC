#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

struct renderer {
	
	int a;

};

struct game_client;

uint32_t renderer_create(struct game_client* game);

uint32_t renderer_resize(struct game_client* game);

uint32_t renderer_destroy(struct game_client* game);

#endif // !RENDERER_H

