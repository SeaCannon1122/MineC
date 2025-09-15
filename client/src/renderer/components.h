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

	struct
	{
		void* data;

		uint32_t resolution;

		uint32_t mappings_count;
		uint32_t* mappings;

		uint32_t bitmaps_count;
		uint8_t* widths;
		void* bitmaps;
	} pixelfont;

};

uint32_t renderer_components_create(struct minec_client* client);
void renderer_components_destroy(struct minec_client* client);

uint32_t renderer_component_pixelfont_create(struct minec_client* client);
void renderer_component_pixelfont_destroy(struct minec_client* client);
uint32_t renderer_backend_component_pixelfont_create(struct minec_client* client);
void renderer_backend_component_pixelfont_destroy(struct minec_client* client);
uint32_t renderer_component_pixelfont_frame(struct minec_client* client);
uint32_t renderer_component_pixelfont_reload_assets(struct minec_client* client);

enum guichar_modifier
{
	GUICHAR_MODIFIER_BOLD_BIT = 1,
	GUICHAR_MODIFIER_ITALIC_BIT = 2,
	GUICHAR_MODIFIER_UNDERLINE_BIT = 4,
	GUICHAR_MODIFIER_STRIKETHROUGH_BIT = 8,
	GUICHAR_MODIFIER_SHADOW_BIT = 16,
	GUICHAR_MODIFIER_BACKGROUND_BIT = 32,
};

struct guichar
{
	uint32_t position[2];
	uint32_t extent[2];
	uint32_t bitmap_index;
	uint32_t bitmap_width;
	uint32_t bitmap_thickness;
	uint32_t modifiers;
	uint32_t color;
	uint32_t bckground_color;
};

struct bitmap_metadata
{
	uint32_t width;
	uint32_t thickness;
};

#define PIXELFONT_MAX_NAME_BUFFER_SIZE 32

#define PIXELFONT_MIN_RESOLUTION 8
#define PIXELFONT_MAX_RESOLUTION 4096

struct pixelfont_header
{
	uint32_t header_section_size;
	uint32_t mappings_section_size;
	uint32_t bitmap_metadata_section_size;
	uint32_t bitmap_data_section_size;

	uint8_t name[PIXELFONT_MAX_NAME_BUFFER_SIZE];
	uint32_t mapping_count;
	uint32_t default_bitmap_index;
	uint32_t resolution;
	uint32_t bitmap_count;
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