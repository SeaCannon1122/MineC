#ifndef GRAPHICS_DEVICE_H
#define GRAPHICS_DEVICE_H

struct game_client;

uint32_t graphics_device_create(struct game_client* game, uint32_t gpu_index);

uint32_t graphics_device_destroy(struct game_client* game);

#endif // !GRAPHICS_DEVICE_H
