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

struct renderer
{
	uint32_t backend_index;
	void* backend_base;
	void* backend_device;
	void* backend_pipelines_resources;

	void* backend_líbrary_handle;
	uint32_t backend_library_load_index;
	struct renderer_backend_interface* backend_interfaces;
	uint32_t backend_count;

	uint32_t fps;

	PixelcharRenderer pixelchar_renderer;
	PixelcharFont pixelchar_fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
};

struct minec_client;

uint32_t renderer_create(struct minec_client* client);
void renderer_destroy(struct minec_client* client);

uint32_t renderer_reload_backend(struct minec_client* client);
uint32_t renderer_switch_backend(struct minec_client* client, uint32_t backend_index);
uint32_t renderer_switch_backend_device(struct minec_client* client, uint32_t device);
void renderer_set_target_fps(struct minec_client* client, uint32_t fps);

#endif
