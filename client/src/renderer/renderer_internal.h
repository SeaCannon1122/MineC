#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H
#define MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H

#include <minec_client.h>

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "backend/opengl/backend_opengl.h"
#endif

union renderer_backend_state
{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	struct renderer_backend_opengl opengl;
#endif
};

struct renderer_internal_state
{
	struct renderer_info_state info_state;

	union renderer_backend_state* backend;
};

struct renderer_backend_api
{
	uint32_t (*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);
};

static struct renderer_backend_api renderer_backend_apis[] = {
	#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy
	},
	#endif
};

static const uint32_t renderer_backend_count = sizeof(renderer_backend_apis) / sizeof(renderer_backend_apis[0]);

#endif
