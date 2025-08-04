#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>

struct minec_client;

uint32_t renderer_backend_create(struct minec_client* client);
void renderer_backend_destroy(struct minec_client* client);

void renderer_backend_frame(struct minec_client* client);

#endif