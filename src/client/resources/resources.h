#pragma once

#ifndef RESOURCES_H
#define RESOURCES_H

#include <general/resource_manager.h>

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
	IMAGE_DIRT,
	IMAGE_STONE,
	IMAGE_BUTTON,
	IMAGE_TEX_2
};

static const uint8_t* const resources_image_tokens[] = {
	"dirt",
	"stone",
	"menu_button",
	"tex2",
};

#define RESOURCES_IMAGES_COUNT sizeof(resources_image_tokens) / sizeof(resources_image_tokens[0])

enum resources_pixelfonts {
	PIXELFONT_DEFAULT,
	PIXELFONT_WIDE,
	PIXELFONT_RECTANGULAR
};

static const uint8_t* const resources_pixelfont_tokens[] = {
	"pixelfont_default",
	"pixelfont_wide",
	"pixelfont_rectangular"
};

#define RESOURCES_PIXELFONTS_COUNT sizeof(resources_pixelfont_tokens) / sizeof(resources_pixelfont_tokens[0])

enum sampler_parameters {
	SAMPLING_DISABLE,
	SAMPLING_ENABLE,
	SAMPLING_LINEAR,
	SAMPLING_NEAREST,
	SAMPLING_REPEAT,
	SAMPLING_CLAMP_TO_EDGE,
	SAMPLING_COMPARE_NEVER,
	SAMPLING_COMPARE_ALWAYS,
	SAMPLING_COMPARE_LESS,
	SAMPLING_COMPARE_EQUAL,
	SAMPLING_COMPARE_GREATER,
	SAMPLING_COMPARE_LESS_EQUAL,
	SAMPLING_COMPARE_GREATER_EQUAL,
	SAMPLING_COMPARE_NOT_EQUAL
};

struct sampler_configuration {
	uint32_t min_filter;
	uint32_t mag_filter;
	uint32_t mipmap_mode;
	uint32_t address_mode_u;
	uint32_t address_mode_v;
	float mip_lod_bias;
	uint32_t anisotropy_enable;
	float max_anisotropy;
	uint32_t compare_enable;
	uint32_t compare_op;
	float min_lod;
	float max_lod;
};

enum resources_sampler_configurations {
	SAMPLER_DEFAULT,
	SAMPLER_SMOOTH,
};

static const uint8_t* const resources_sampler_tokens[] = {
	"sampler_default",
	"sampler_smooth"
};

#define RESOURCES_SAMPLERS_COUNT sizeof(resources_sampler_tokens) / sizeof(resources_sampler_tokens[0])

struct resource_state {

	uint8_t resource_manager_root_path[256];
	struct resource_manager resource_manager;

	//images
	struct resource_manager_image image_atlas[RESOURCES_IMAGES_COUNT];

	//samplers
	struct sampler_configuration sampler_atlas[RESOURCES_SAMPLERS_COUNT];

	//pixelfonts
	struct pixel_font* pixelfont_atlas[RESOURCES_PIXELFONTS_COUNT];
	struct pixel_font* fallback_pixelfont;

};

struct game_client;

uint32_t resources_create(struct game_client* game, uint8_t* resource_path);

uint32_t resources_destroy(struct game_client* game);

uint32_t resources_reload(struct game_client* game);

#endif // !RESOURCES_H
