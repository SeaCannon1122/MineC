#pragma once

#ifndef MINEC_CLIENT_RENDERER_FRONTEND_FRONTEND_H
#define MINEC_CLIENT_RENDERER_FRONTEND_FRONTEND_H

#include <stdint.h>
#include "backend/backend.h"

#define RENDERER_MENU_PIXELCHAR_BUFFER_LENGTH 256

#define RENDERER_PIXELCHAR_BUFFER_LENGTH RENDERER_MENU_PIXELCHAR_BUFFER_LENGTH

struct renderer_rendering_settings
{
	bool order_create_new_destroy_old;
	uint32_t fov;
	uint32_t gui_scale;
	uint32_t render_distance;
};

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

enum menu_texture_enum
{
	MENU_TEXTURE_BACKGROUND,
	MENU_TEXTURE_s_COUNT
};

static const uint8_t* menu_texture_names[1] =
{
	"minec/textures/menu/background.png"
};

struct menu_texture
{
	uint32_t width;
	uint32_t height;
	uint32_t original_channels;
	uint32_t* data;
};

struct renderer_components
{
	struct renderer_backend backend;

	bool backend_base_created;
	bool backend_device_created;
	bool backend_swapchain_created;

	struct
	{
		struct menu_texture tetxures[MENU_TEXTURE_s_COUNT];
	} menu;
};

struct minec_client;

uint32_t renderer_components_create(struct minec_client* client);
void renderer_components_destroy(struct minec_client* client);

uint32_t renderer_frame(struct minec_client* client);

#endif

#endif