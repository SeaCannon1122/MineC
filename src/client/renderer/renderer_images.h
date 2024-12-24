#pragma once

#ifndef RENDERER_IMAGES_H
#define RENDERER_IMAGES_H

#include <stdint.h>
#include "client/resources/resources.h"

enum sampling_configuraion_parameters {
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

struct renderer_sampling_configuration {
    uint32_t mag_filter;
    uint32_t min_filter;
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

enum renderer_sampling_configuration_enum {
    SAMPLING_CONFIGURATION_DEFAULT,
};

static struct renderer_sampling_configuration renderer_sampling_configurations[] = {
    {SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_REPEAT, SAMPLING_REPEAT, 0.f, SAMPLING_DISABLE, 0.f, SAMPLING_DISABLE, 0, 0.f, 8.f}
};

#define RENDERER_SAMPLING_CONFIGURATIONS_COUNT sizeof(renderer_sampling_configurations) / sizeof(renderer_sampling_configurations[0])

enum renderer_image_enum {
    RENDERER_IMAGE_INGAME_DIRT,
    RENDERER_IMAGE_INGAME_STONE,
    RENDERER_IMAGE_INAGME_TEX_1,
    RENDERER_IMAGE_INGAME_TEX_2,
};

struct renderer_image {
	uint32_t resource_image_index;
	uint32_t sampling_configuarion;
};

static struct renderer_image renderer_images[] = {
    {RESOURCE_IMAGE_DIRT, SAMPLING_CONFIGURATION_DEFAULT},
    {RESOURCE_IMAGE_STONE, SAMPLING_CONFIGURATION_DEFAULT},
    {RESOURCE_IMAGE_BUTTON, SAMPLING_CONFIGURATION_DEFAULT},
    {RESOURCE_IMAGE_TEX_2, SAMPLING_CONFIGURATION_DEFAULT}
};

#define RENDERER_IMAGES_COUNT sizeof(renderer_images) / sizeof(renderer_images[0])

#endif