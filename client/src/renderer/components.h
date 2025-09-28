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

	struct
	{
		bool base;
		bool device;
		bool swapchain;
	} backend_create_flags;
	bool frontend_created;

	bool backend_recreate;

	struct
	{
		struct menu_texture tetxures[MENU_TEXTURE_s_COUNT];
	} menu;


};

uint32_t renderer_components_create(struct minec_client* client);
void renderer_components_destroy(struct minec_client* client);

uint32_t renderer_component_pixelfont_create(struct minec_client* client);
void renderer_component_pixelfont_destroy(struct minec_client* client);
uint32_t renderer_backend_component_pixelfont_create(struct minec_client* client);
void renderer_backend_component_pixelfont_destroy(struct minec_client* client);
uint32_t renderer_component_pixelfont_frame(struct minec_client* client);
uint32_t renderer_component_pixelfont_reload_assets(struct minec_client* client);

struct guichar_vertex_attributes
{
	uint32_t position[2];
	uint32_t extent[2];
	uint32_t bitmap_index;
	uint32_t bitmap_width;
	uint32_t bitmap_thickness;
	uint32_t modifiers;
	uint32_t color;
	uint32_t background_color;
};

uint32_t renderer_component_gui_text_create(struct minec_client* client);
void renderer_component_gui_text_destroy(struct minec_client* client);
uint32_t renderer_backend_component_gui_text_create(struct minec_client* client);
void renderer_backend_component_gui_text_destroy(struct minec_client* client);
uint32_t renderer_component_gui_text_frame(struct minec_client* client);
uint32_t renderer_component_gui_text_reload_assets(struct minec_client* client);

uint32_t renderer_component_menu_gui_create(struct minec_client* client);
void renderer_component_menu_gui_destroy(struct minec_client* client);
uint32_t renderer_backend_component_menu_gui_create(struct minec_client* client);
void renderer_backend_component_menu_gui_destroy(struct minec_client* client);
uint32_t renderer_component_menu_gui_frame(struct minec_client* client);
uint32_t renderer_component_menu_gui_reload_assets(struct minec_client* client);

#endif

#endif