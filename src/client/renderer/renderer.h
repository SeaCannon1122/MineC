#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include "backend/renderer_backend.h"
#include "backend/vulkan/vulkan_backend.h"
#include <stdint.h>

struct renderer_rectangle {

	int16_t x[4];
	int16_t y[4];
	float u[4];
	float v[4];

	int16_t image_index;
	int16_t sampler_index;
};

struct renderer_state {

	struct renderer_backend backend;

	int a;
};

struct game_client;

uint32_t renderer_create(struct game_client* game);

uint32_t renderer_destroy(struct game_client* game);

uint32_t renderer_render(struct game_client* game);

uint32_t renderer_list_gpus(struct game_client* game);

uint32_t renderer_use_gpu(struct game_client* game, uint32_t gpu_index);

uint32_t renderer_reload_resources(struct game_client* game);

#endif // !RENDERER_H
