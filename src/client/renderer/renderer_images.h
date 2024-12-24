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
    SAMPLING_CONFIGURATION_SMOOTH,
};

static struct renderer_sampling_configuration renderer_sampling_configurations[] = {
    {SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_REPEAT, SAMPLING_REPEAT, 0.f, SAMPLING_DISABLE, 0.f, SAMPLING_DISABLE, 0, 0.f, 0.f},
    {SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_NEAREST, SAMPLING_REPEAT, SAMPLING_REPEAT, 0.f, SAMPLING_DISABLE, 0.f, SAMPLING_DISABLE, 0, 0.f, 8.f}
};

#define RENDERER_SAMPLING_CONFIGURATIONS_COUNT sizeof(renderer_sampling_configurations) / sizeof(renderer_sampling_configurations[0])

#endif