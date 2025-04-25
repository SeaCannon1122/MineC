#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <pixelchar/pixelchar.h>

#include "backend/renderer_backend.h"

struct renderer_state
{
	struct
	{
		int a;
#ifdef _MINEC_CLIENT_RENDERER_BACKEND_VULKAN
		struct backend_vulkan vulkan;
#endif
		
	} backend;

	PixelcharRenderer pixelchar_renderer;
};

struct minec_client;

void renderer_create(struct minec_client* client);
void renderer_destroy(struct minec_client* client);

void renderer_render(struct minec_client* client);

void renderer_resize(struct minec_client* client);

void renderer_reload_resources(struct minec_client* client);

#endif
