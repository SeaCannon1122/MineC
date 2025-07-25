#pragma once

#ifndef MINEC_CLIENT_RENDERER_FRONTEND_H
#define MINEC_CLIENT_RENDERER_FRONTEND_H

#include <stdint.h>

struct minec_client;

uint32_t renderer_frontend_create(struct minec_client* client);
void renderer_frontend_destroy(struct minec_client* client);

void renderer_frontend_frame(struct minec_client* client);

void renderer_frontend_reload_resources(struct minec_client* client);
void renderer_frontend_set_gui_scale(struct minec_client* client, uint32_t scale);
void renderer_frontend_set_fov(struct minec_client* client, uint32_t fov);
void renderer_frontend_set_render_distance(struct minec_client* client, uint32_t render_distance);

#endif