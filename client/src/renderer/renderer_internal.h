#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H
#define MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H

#include <minec_client.h>

#include "backend.h"

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "backend/opengl/backend_opengl.h"
#endif

#define RENDERER (*client->renderer.state)
#define BACKEND backend_memory[client->renderer.state->backend_memory_index]

struct renderer_backend
{
	struct renderer_backend_device_info device_infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];
	uint32_t device_count;

	struct
	{
		uint32_t backend_device_index;
		bool vsync;
		uint32_t max_mipmap_level_count;
		uint32_t fps;
	} settings;

	union renderer_backend_internal_state
	{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
		struct renderer_backend_opengl opengl;
#endif
	} state;
};

struct renderer_internal_state
{
	void* thread_handle;

	bool crashing;

	struct renderer_backend backend_memory[2];
	uint32_t backend_memory_index;

	struct
	{
		uint32_t gui_scale;
		uint32_t fov;
		uint32_t render_distance;
		uint32_t backend_index;
	} settings;

	struct
	{
		struct
		{
			mutex_t mutex;

			uint32_t gui_scale;
			uint32_t fov;
			uint32_t render_distance;
			uint32_t backend_index;
			uint32_t backend_device_index;
			bool vsync;
			uint32_t fps;
			uint32_t max_mipmap_level_count;
		} settings;

		struct
		{
			mutex_t mutex;
			bool changed;
			struct renderer_info_state state;
		} info;

		atomic_bool active;
		atomic_bool created;
	} public;
};

#endif
