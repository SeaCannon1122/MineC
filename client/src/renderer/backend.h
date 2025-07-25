#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>

struct minec_client;

uint32_t renderer_backend_create(struct minec_client* client);
void renderer_backend_destroy(struct minec_client* client);

void renderer_backend_frame(struct minec_client* client);

void renderer_backend_switch(struct minec_client* client, uint32_t index);
void renderer_backend_switch_device(struct minec_client* client, uint32_t device_index);
void renderer_backend_reload_resources(struct minec_client* client);
void renderer_backend_set_vsync(struct minec_client* client, bool vsync);
void renderer_backend_set_fps(struct minec_client* client, uint32_t fps);
void renderer_backend_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);
#endif