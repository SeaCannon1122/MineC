#pragma once

#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

#ifdef CLIENT_GRAPHICS_API_VULKAN

#include "vulkan/renderer_vulkan.h"

static uint8_t* shader_token_prefix = "vk_";

#elif CLIENT_GRAPHICS_API_OPENGL

//opengl in the future???

#endif // CLIENT_GRAPHICS_API_VULKAN


uint32_t renderer_backend_list_gpus(struct game_client* game);

uint32_t renderer_backend_use_gpu(struct game_client* game, uint32_t gpu_index);

uint32_t renderer_backend_create(struct game_client* game);
uint32_t renderer_backend_destroy(struct game_client* game);

uint32_t renderer_backend_resize(struct game_client* game);

uint32_t renderer_backend_load_resources(struct game_client* game);
uint32_t renderer_backend_unload_resources(struct game_client* game);

#endif // RENDERER_BACKEND_H
