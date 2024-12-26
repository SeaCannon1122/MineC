#pragma once

#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

#ifdef CLIENT_GRAPHICS_API_VULKAN

#include "vulkan/vulkan_backend.h"

#elif CLIENT_GRAPHICS_API_OPENGL

//opengl in the future???

#endif // CLIENT_GRAPHICS_API_VULKAN

struct game_client;

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index);

uint32_t renderer_backend_create(struct game_client* game);
uint32_t renderer_backend_destroy(struct game_client* game);

uint32_t renderer_backend_resize(struct game_client* game);

uint32_t renderer_backend_load_resources(struct game_client* game);
uint32_t renderer_backend_unload_resources(struct game_client* game);

uint32_t renderer_backend_set_pixel_chars(struct game_client* game, struct pixel_render_char* chars, uint32_t chars_count);
uint32_t renderer_backend_set_rectangles(struct game_client* game, struct renderer_rectangle* rectangles, uint32_t rectangles_count);

uint32_t renderer_backend_render(struct game_client* game);

#endif // RENDERER_BACKEND_H
