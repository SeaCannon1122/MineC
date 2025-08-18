#pragma once

#ifndef PIXELCHAR_BACKEND_OPENGL_H
#define PIXELCHAR_BACKEND_OPENGL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

#define PIXELCHAR_BACKEND_OPENGL_MAX_RESOURCE_FRAME_COUNT 128

PixelcharResult pixelcharRendererBackendOpenGLInitialize(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameCount,
	void* (*pfnglGetProcAddress)(uint8_t*),
	uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceLength,
	uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	void (*shaderErrorLogFunction)(void* userParam, uint8_t* logText),
	void* shaderErrorLogFunctionUserParam
);

void pixelcharRendererBackendOpenGLDeinitialize(PixelcharRenderer renderer, uint32_t backendSlotIndex);

void pixelcharRendererBackendOpenGLRender(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	uint32_t resourceFrameIndex,
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
