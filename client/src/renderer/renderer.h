#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#include <pixelchar/pixelchar.h>

#include "backend/backend.h"

static void renderer_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	minec_client_log_v(client, "[RENDERER]", message, args);
	va_end(args);
}

struct renderer_backend_global_state
{
	void* líbrary_handle;
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
	bool created;
};

struct renderer
{

	struct
	{
		struct renderer_backend_global_state global;
		struct renderer_backend_base_state base;
		struct renderer_backend_device_state device;
		struct renderer_backend_pipelines_resources_state pipelines_resources;
	} backend;
	
	uint32_t backend_library_load_index;
	uint8_t* backend_library_paths[3];

	PixelcharRenderer pixelchar_renderer;
	PixelcharFont pixelchar_fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];

	void* thread_handle;
	mutex_t thread_mutex;
	atomic_(bool) thread_should_close;
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
