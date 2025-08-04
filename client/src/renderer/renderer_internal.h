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


#define ACCESS_INFO_STATE(changes) {mutex_lock(&RENDERER.public.info.mutex); changes mutex_unlock(&RENDERER.public.info.mutex);}
#define ACCESS_REQUEST_STATE(changes) {mutex_lock(&RENDERER.public.request.mutex); changes mutex_unlock(&RENDERER.public.request.mutex);}

#define MINEC_CLIENT_PIXELCHAR_RENDERER_QUEUE_LENGTH 1024

struct renderer_backend
{
	struct renderer_backend_device_infos device_infos;
	bool device_infos_changed;

	struct renderer_backend_settings settings;

	union renderer_backend_internal_state
	{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
		struct renderer_backend_opengl opengl;
#endif
	} state;

	uint32_t pixelchar_slots[2];
	uint32_t pixelchar_slot_index;
};

struct renderer_frontend
{
	struct renderer_frontend_settings settings;

	PixelcharRenderer pixelchar_renderer;

	struct
	{
		float last_frame_time;

		uint32_t width;
		uint32_t height;
	} frame_info;
};

struct renderer_internal_state
{
	void* thread_handle;

	bool crashing;

	struct renderer_frontend frontend;
	struct renderer_backend backend;
	bool backend_exists;

	struct renderer_other_settings settings;

	struct
	{
		struct
		{
			mutex_t mutex;
			struct renderer_settings settings;
			bool reload_resources;
			bool reload_resources_detected;
			bool restart;
		} request;

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
