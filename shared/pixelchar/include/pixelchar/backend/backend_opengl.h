#pragma once

#ifndef PIXELCHAR_BACKEND_OPENGL_H
#define PIXELCHAR_BACKEND_OPENGL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
	void* (*pfnglGetProcAddress)(uint8_t*),
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

void pixelcharRendererBackendOpenGLDeinitialize(PixelcharRenderer renderer);

PixelcharResult pixelcharRendererBackendOpenGLRender(
	PixelcharRenderer renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);

#ifdef __cplusplus
}
#endif

#endif
