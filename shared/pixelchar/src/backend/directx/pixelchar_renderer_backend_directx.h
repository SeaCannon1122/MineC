#pragma once

#ifndef PIXELCHAR_RENDERER_BACKEND_DIRECTX_H
#define PIXELCHAR_RENDERER_BACKEND_DIRECTX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct pixelchar_renderer_backend_directx
{
	int a;
};

uint32_t pixelchar_renderer_backend_directx_init(
	struct pixelchar_renderer* pcr,
	uint8_t* vertex_shader_custom_source,
	size_t vertex_shader_custom_source_size,
	uint8_t* fragment_shader_custom_source,
	size_t fragment_shader_custom_source_size
);

void pixelchar_renderer_backend_directx_deinit(struct pixelchar_renderer* pcr);

void pixelchar_renderer_backend_directx_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height, float shadow_devisor_r, float shadow_devisor_g, float shadow_devisor_b, float shadow_devisor_a);

#ifdef __cplusplus
}
#endif

#endif