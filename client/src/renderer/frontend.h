#pragma once

#ifndef MINEC_CLIENT_RENDERER_FRONTEND_H
#define MINEC_CLIENT_RENDERER_FRONTEND_H

#include <stdint.h>

struct minec_client;

uint32_t renderer_frontend_create(struct minec_client* client);
void renderer_frontend_destroy(struct minec_client* client);

void renderer_frontend_frame(struct minec_client* client);


#endif