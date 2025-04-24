#pragma once

#ifndef BACKEND_H
#define BACKEND_H

typedef enum _pixelchar_backend
{
	PIXELCHAR_BACKEND_OPENGL,
	PIXELCHAR_BACKEND_VULKAN,
	PIXELCHAR_BACKEND_DIRECT3D11,
	PIXELCHAR_BACKEND_DIRECT3D12,
	PIXELCHAR_BACKEND_METAL,
	PIXELCHAR_BACKEND_s_COUNT
} _pixelchar_backend;

static void (*pixelchar_renderer_backend_deinitialize_functions[])(struct pixelchar_renderer*) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	pixelchar_renderer_backend_vulkan_deinit,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	pixelchar_renderer_backend_opengl_deinit,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT3d11
	pixelchar_renderer_backend_directx_deinit,
#else
	NULL,
#endif
};

static void (*_pixelchar_font_backend_reference_init_functions[])(struct pixelchar_font*) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_font_backend_vulkan_reference_init,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_font_backend_opengl_reference_init,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECTX
	_pixelchar_font_backend_directx_reference_init,
#else
	NULL,
#endif
};

static uint32_t(*_pixelchar_font_backend_reference_add_functions[])(struct pixelchar_font*, struct pixelchar_renderer*, uint32_t) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_font_backend_vulkan_reference_add,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_font_backend_opengl_reference_add,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECTX
	_pixelchar_font_backend_directx_reference_add,
#else
	NULL,
#endif
};

static void (*_pixelchar_font_backend_reference_subtract_functions[])(struct pixelchar_font*) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_font_backend_vulkan_reference_subtract,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_font_backend_opengl_reference_subtract,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECTX
	_pixelchar_font_backend_directx_reference_subtract,
#else
	NULL,
#endif
};

#endif
