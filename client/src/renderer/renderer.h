#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#include <pixelchar/pixelchar.h>

#include "backend/backend.h"

struct renderer_backend_global_state
{
	void* líbrary_handle;
	uint32_t library_load_index;
	struct renderer_backend_interface* interfaces;
	uint32_t backend_count;
	uint8_t** backend_names;
	uint32_t backend_index;
};

struct renderer_backend_base_state
{
	void* base;

	uint8_t** device_infos;
	uint32_t device_index;
	uint32_t device_count;
};

struct renderer_backend_device_state
{
	void* device;

	uint32_t fps;
};

struct renderer_backend_pipelines_resources_state
{
	void* pipelines_resources;

	uint32_t pcr_backend_index;
};

enum renderer_request_flag
{
	RENDERER_REQUEST_RENDER,
	RENDERER_REQUEST_CLOSE,
	RENDERER_REQUEST_HALT
};

struct renderer
{
	atomic_(uint32_t) request_flag;
	mutex_t mutex;

	struct
	{
		struct renderer_backend_global_state global;
		struct renderer_backend_base_state base;
		struct renderer_backend_device_state device;
		struct renderer_backend_pipelines_resources_state pipelines_resources;
	} backend;

	uint8_t* backend_library_paths[3];

	PixelcharRenderer pixelchar_renderer;

	struct rendering_thread_state
	{
		void* handle;

		struct
		{
			float time;
			uint32_t index;

		} frame_info;
	} thread_state;
};

struct minec_client;

uint32_t renderer_create(
	struct minec_client* client,
	uint32_t* backend_index,
	uint32_t* backend_count,
	uint8_t*** backend_names,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos,
	uint32_t fps
);

void renderer_destroy(struct minec_client* client);

uint32_t renderer_reload_backend(
	struct minec_client* client,
	uint32_t* backend_index,
	uint32_t* backend_count,
	uint8_t*** backend_names,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos
);

uint32_t renderer_switch_backend(
	struct minec_client* client,
	uint32_t backend_index,
	uint32_t* device_index,
	uint32_t* device_count,
	uint8_t*** device_infos
);

uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device);
uint32_t renderer_set_target_fps(struct minec_client* client, uint32_t fps);

#endif
