#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H
#define MINEC_CLIENT_RENDERER_RENDERER_INTERNAL_H

#include <minec_client.h>
#include <pixelchar/pixelchar.h>

#include "frontend.h"
#include "backend.h"

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "backend/opengl/backend_opengl.h"
#endif

#define RENDERER (*client->renderer.state)
#define BACKEND backend_memory[client->renderer.state->backend_memory_index]

#define ACCESS_INFO_STATE(changes) {mutex_lock(&RENDERER.public.info.mutex); changes mutex_unlock(&RENDERER.public.info.mutex);}
#define ACCESS_REQUESTED_SETTINGS(changes) {mutex_lock(&RENDERER.public.requested_settings_mutex); changes mutex_unlock(&RENDERER.public.requested_settings_mutex);}

#define MINEC_CLIENT_PIXELCHAR_RENDERER_QUEUE_LENGTH 1024

struct renderer_backend
{
	struct renderer_backend_device_info device_infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];
	uint32_t device_count;

	struct
	{
		uint32_t backend_device_index;
		uint32_t fps;
		bool vsync;
		uint32_t max_mipmap_level_count;
		
	} settings;

	union renderer_backend_internal_state
	{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
		struct renderer_backend_opengl opengl;
#endif
	} state;

	uint32_t pixelchar_slots[2];
};

struct renderer_internal_state
{
	void* thread_handle;

	bool crashing;

	struct renderer_backend backend_memory[2];
	uint32_t backend_memory_index;

	struct
	{
		PixelcharRenderer pixelchar_renderer;
	} frontend;

	struct
	{
		uint32_t gui_scale;
		uint32_t fov;
		uint32_t render_distance;
		uint32_t backend_index;
	} settings;

	struct
	{
		struct renderer_settings requested_settings;
		mutex_t requested_settings_mutex;

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
