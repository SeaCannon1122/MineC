#pragma once

#ifndef RESOURCES_H
#define RESOURCES_H

#include "general/resource_manager.h"

static const uint32_t default_image[] = {
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
	0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,   0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,

	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
	0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,   0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef, 0xff00f8ef,
};
static const uint32_t default_image_width = 16;
static const uint32_t default_image_height = 16;

enum resources_images {
	IMAGE_MENU_BACKGROUND,
	IMAGE_MENU_BUTTON,
	IMAGE_MENU_BUTTON_HOVER,
	IMAGE_MENU_BUTTON_DISABLED,
	IMAGE_MENU_BUTTON_SHORT,
	IMAGE_MENU_BUTTON_HOVER_SHORT,
	IMAGE_MENU_BUTTON_DISABLED_SHORT,
	IMAGE_MENU_TEXTFIELD,
	IMAGE_MENU_TEXTFIELD_SELECTED,
	IMAGE_DIRT,
	IMAGE_STONE,
	IMAGE_TEX_2
};

static const uint8_t* const resources_image_tokens[] = {
	"menu_background",
	"menu_button",
	"menu_button_hover",
	"menu_button_disabled",
	"menu_button_short",
	"menu_button_hover_short",
	"menu_button_disabled_short",
	"menu_textfield",
	"menu_textfield_selected",
	"dirt",
	"stone",
	"tex2",
};

#define RESOURCES_IMAGES_COUNT sizeof(resources_image_tokens) / sizeof(resources_image_tokens[0])

enum resources_pixelfonts {
	PIXELFONT_DEFAULT,
	PIXELFONT_WIDE,
	PIXELFONT_SMOOTH
};

static const uint8_t* const resources_pixelfont_tokens[] = {
	"pixelfont_default",
	"pixelfont_wide",
	"pixelfont_smooth",
};

#define RESOURCES_PIXELFONTS_COUNT sizeof(resources_pixelfont_tokens) / sizeof(resources_pixelfont_tokens[0])

enum resources_key_value_maps {
	RESOURCES_KVM_SAMPLER_DEFAULT,
	RESOURCES_KVM_SAMPLER_SMOOTH,
};

static const uint8_t* const resources_key_value_map_tokens[] = {
	"sampler_default",
	"sampler_smooth"
};

#define RESOURCES_KVM_COUNT sizeof(resources_key_value_map_tokens) / sizeof(resources_key_value_map_tokens[0])

enum resources_shaders {
	//vulkan shaders
	SHADER_VULKAN_PIXELCHAR_VERTEX,
	SHADER_VULKAN_PIXELCHAR_FRAGMENT,
	SHADER_VULKAN_RECTANGLES_VERTEX,
	SHADER_VULKAN_RECTANGLES_FRAGMENT,

	//opengl shaders
};

static const uint8_t* const resources_shader_tokens[] = {
	"vulkan_pixelchar_vertex",
	"vulkan_pixelchar_fragment",
	"vulkan_rectangles_vertex",
	"vulkan_rectangles_fragment",
};

#define RESOURCES_SHADERS_COUNT sizeof(resources_shader_tokens) / sizeof(resources_shader_tokens[0])

struct resource_state {

	uint8_t resource_manager_root_path[256];
	struct resource_manager resource_manager;

	//images
	struct resource_manager_image image_atlas[RESOURCES_IMAGES_COUNT];

	//key value maps
	void* key_value_map_atlas[RESOURCES_KVM_COUNT];

	//pixelfonts
	struct pixel_font* pixelfont_atlas[RESOURCES_PIXELFONTS_COUNT];
	struct pixel_font* fallback_pixelfont;

	//shaders
	struct resource_manager_binary shader_atlas[RESOURCES_SHADERS_COUNT];

};

struct game_client;

uint32_t resources_create(struct game_client* game, uint8_t* resource_path);

uint32_t resources_destroy(struct game_client* game);

uint32_t resources_reload(struct game_client* game);

#endif // !RESOURCES_H
