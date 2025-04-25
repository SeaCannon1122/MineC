#pragma once

#ifndef BACKEND_H
#define BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

typedef enum _pixelchar_backend
{
	PIXELCHAR_BACKEND_OPENGL,
	PIXELCHAR_BACKEND_VULKAN,
	PIXELCHAR_BACKEND_DIRECT3D11,
	PIXELCHAR_BACKEND_DIRECT3D12,
	PIXELCHAR_BACKEND_METAL,
	PIXELCHAR_BACKEND_s_COUNT
} _pixelchar_backend;

void pixelcharRendererBackendOpenGLDeinitialize(PixelcharRenderer renderer);
void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer);
void pixelcharRendererBackendDirect3D11Deinitialize(PixelcharRenderer renderer);
void pixelcharRendererBackendDirect3D12Deinitialize(PixelcharRenderer renderer);
void pixelcharRendererBackendMetalDeinitialize(PixelcharRenderer renderer);

static void (*_renderer_backend_deinitialize_functions[])(PixelcharRenderer renderer) = {
#ifdef _PIXELCHAR_BACKEND_OPENGL
	pixelcharRendererBackendOpenGLDeinitialize,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_VULKAN
	pixelcharRendererBackendVulkanDeinitialize,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D11
	pixelcharRendererBackendDirect3D11Deinitialize,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D12
	pixelcharRendererBackendDirect3D12Deinitialize,
#else
	NULL,
#endif
	#ifdef _PIXELCHAR_BACKEND_METAL
	pixelcharRendererBackendMetalDeinitialize,
#else
	NULL,
#endif
};

PixelcharResult _font_backend_opengl_add_reference(PixelcharRenderer renderer, uint32_t font_index);
PixelcharResult _font_backend_vulkan_add_reference(PixelcharRenderer renderer, uint32_t font_index);
PixelcharResult _font_backend_direct3d11_add_reference(PixelcharRenderer renderer, uint32_t font_index);
PixelcharResult _font_backend_direct3d12_add_reference(PixelcharRenderer renderer, uint32_t font_index);
PixelcharResult _font_backend_metal_add_reference(PixelcharRenderer renderer, uint32_t font_index);

static PixelcharResult(*_font_backend_add_reference_functions[])(PixelcharRenderer renderer, uint32_t font_index) = {
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_font_backend_opengl_add_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_font_backend_vulkan_add_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D11
	_font_backend_direct3d11_add_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D12
	_font_backend_direct3d12_add_reference,
#else
	NULL,
#endif
	#ifdef _PIXELCHAR_BACKEND_METAL
	_font_backend_metal_add_reference,
#else
	NULL,
#endif
};

void _font_backend_opengl_sub_reference(PixelcharRenderer renderer, uint32_t font_index);
void _font_backend_vulkan_sub_reference(PixelcharRenderer renderer, uint32_t font_index);
void _font_backend_direct3d11_sub_reference(PixelcharRenderer renderer, uint32_t font_index);
void _font_backend_direct3d12_sub_reference(PixelcharRenderer renderer, uint32_t font_index);
void _font_backend_metal_sub_reference(PixelcharRenderer renderer, uint32_t font_index);

static void (*_font_backend_sub_reference_functions[])(PixelcharRenderer renderer, uint32_t font_index) = {
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_font_backend_opengl_sub_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_font_backend_vulkan_sub_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D11
	_font_backend_direct3d11_sub_reference,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3D12
	_font_backend_direct3d12_sub_reference,
#else
	NULL,
#endif
	#ifdef _PIXELCHAR_BACKEND_METAL
	_font_backend_metal_sub_reference,
#else
	NULL,
#endif
};

#ifdef __cplusplus
}
#endif

#endif
