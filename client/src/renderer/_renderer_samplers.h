#pragma once

#ifndef RENDERER_SAMPLERS
#define RENDERER_SAMPLERS

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

enum sampler_configurations {
	SAMPLER_DEFAULT,
	SAMPLER_SMOOTH,
};

static const uint32_t sampler_configurations_key_value_maps[] = {
	KVM_SAMPLER_DEFAULT,
	KVM_SAMPLER_SMOOTH,
};

#define SAMPLERS_COUNT sizeof(sampler_configurations_key_value_maps) / sizeof(sampler_configurations_key_value_maps[0])

#endif // !RENDERER_SAMPLERS