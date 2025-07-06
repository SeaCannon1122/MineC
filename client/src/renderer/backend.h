#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>

struct minec_client;

uint32_t renderer_backend_initialize(struct minec_client* client);
void renderer_backend_deinitialize(struct minec_client* client);

#endif