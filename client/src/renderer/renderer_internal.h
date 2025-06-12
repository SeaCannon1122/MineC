#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H
#define MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H

#include <minec_client.h>

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "backend/opengl/backend_opengl.h"
#endif

struct renderer_internal_state
{
	struct renderer_info_state info_state;

	struct
	{
		union renderer_backend_state
		{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
			struct renderer_backend_opengl opengl;
#endif
		};

	} backend;

	uint32_t backend_index;
};

struct renderer_backend_settings
{
	uint32_t backend_device_index;
	bool vsync;
	uint32_t max_mipmap_level_count;
};

struct renderer_backend_api
{
	uint32_t (*create)(
		struct minec_client* client,
		union renderer_backend_state* backend_state,
		struct renderer_backend_settings* settings,
		struct renderer_backend_info* backend_info,
		struct renderer_backend_device_info* device_infos
	);

	void (*destroy)(
		struct minec_client* client,
		union renderer_backend_state* backend_state
	);
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
