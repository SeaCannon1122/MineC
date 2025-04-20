#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <pixelchar/pixelchar.h>

struct renderer_state
{
	struct
	{
		int a;
	} backend;

	struct pixelchar_renderer pixelchar_renderer;
};

struct minec_client;

void renderer_create(struct minec_client* client);
void renderer_destroy(struct minec_client* client);

void renderer_render(struct minec_client* client);

void renderer_resize(struct minec_client* client);

void renderer_reload_resources(struct minec_client* client);

#endif
