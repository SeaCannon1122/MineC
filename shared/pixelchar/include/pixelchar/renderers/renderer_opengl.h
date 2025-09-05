#pragma once

#ifndef PIXELCHAR_BACKEND_OPENGL_H
#define PIXELCHAR_BACKEND_OPENGL_H


#include <pixelchar/pixelchar.h>

#define PIXELCHAR_RENDERER_OPENGL_MAX_RESOURCE_FRAME_COUNT 4

typedef struct PixelcharRendererOpenGL_T* PixelcharRendererOpenGL;

bool PixelcharRendererOpenGLCreate(
	void* (*pfnglGetProcAddress)(void* userParam, uint8_t*),
	void* pfnglGetProcAddressUserParam,
	uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceLength,
	uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceLength,
	void (*shaderErrorLogFunction)(void* userParam, uint8_t* logText),
	void* shaderErrorLogFunctionUserParam,
	uint32_t resourceFrameCount,
	uint32_t maxResourceFrameCharacterCount,
	PixelcharRendererOpenGL* pRenderer
);
void PixelcharRendererOpenGLDestroy(PixelcharRendererOpenGL renderer);
bool PixelcharRendererOpenGLUseFont(PixelcharRendererOpenGL renderer, PixelcharFont font, uint32_t fontIndex);

void PixelcharRendererVulkanUpdateRenderingData(
	PixelcharRendererOpenGL renderer,
	Pixelchar* pCharacters,
	uint32_t characterCount,
	uint32_t resourceFrameIndex
);

void PixelcharRendererVulkanResetResourceFrame(PixelcharRendererOpenGL renderer, uint32_t resourceFrameIndex);

void PixelcharRendererVulkanRender(
	PixelcharRendererOpenGL renderer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);


#endif
