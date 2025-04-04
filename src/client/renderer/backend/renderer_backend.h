#pragma once

#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

enum renderer_backend_type
{
#ifdef CLIENT_GRAPHICS_API_VULKAN
	RENDERER_BACKEND_VULKAN,
#endif

#ifdef CLIENT_GRAPHICS_API_OPENGL
	RENDERER_BACKEND_OPENGL,
#endif

#ifdef CLIENT_GRAPHICS_API_DIRECT
	RENDERER_BACKEND_DIRECT,
#endif

	RENDERER_BACKEND_MAX_ENUM,
};

#include "vulkan/vulkan_backend.h"

struct game_client;
struct renderer_rectangle;

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index);

uint32_t renderer_backend_create(struct game_client* game);
uint32_t renderer_backend_destroy(struct game_client* game);

uint32_t renderer_backend_resize(struct game_client* game);

uint32_t renderer_backend_load_resources(struct game_client* game);
uint32_t renderer_backend_unload_resources(struct game_client* game);

uint32_t renderer_backend_add_pixel_chars(struct game_client* game, struct pixel_char* chars, uint32_t chars_count);
uint32_t renderer_backend_add_rectangles(struct game_client* game, struct renderer_rectangle* rectangles, uint32_t rectangles_count);

uint32_t renderer_backend_render(struct game_client* game);

#endif // RENDERER_BACKEND_H
