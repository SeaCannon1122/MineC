#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#include <pixelchar/pixelchar.h>

#include "backend/backend.h"

static void renderer_log(struct minec_client* client, uint8_t* string, ...)
{
	va_list args;
	va_start(args, string);
	minec_client_log_v(client, "[RENDERER] %s", args);
	minec_client_log(client, "a");
	va_end(args);
}

struct renderer
{
	uint32_t backend_index;
	void* backend[2];
	uint32_t backend_slot_index;

	void* backend_líbrary_handle;
	struct renderer_backend_interface* backend_interfaces;
	uint32_t backend_interface_count;

	PixelcharRenderer pixelchar_renderer;
	PixelcharFont pixelchar_fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
};

struct minec_client;

uint32_t renderer_create(struct minec_client* client);
void renderer_destroy(struct minec_client* client);

uint32_t renderer_switch_backend(struct minec_client* client, uint32_t backend_index);
void renderer_set_target_fps(struct minec_client* client, uint32_t fps);

#endif
