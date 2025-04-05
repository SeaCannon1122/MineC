#pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#include <stdlib.h>
#include <string.h>

#include "pixelchar_renderer.h"
#include "pixelfont.h"
#include "backend/pixelchar_renderer_backend.h"

extern PIXELCHAR_DEBUG_CALLBACK_FUNCTION debug_callback_function;

#define _DEBUG_CALLBACK_WARNING(msg) if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING, msg)

#define _DEBUG_CALLBACK_ERROR(msg) if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR, msg)

#define _DEBUG_CALLBACK_ERROR_RETURN(msg) do {\
if (debug_callback_function) {debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR, msg);}\
return PIXELCHAR_FAILED;\
} while(0)

#define _DEBUG_CALLBACK_CRITICAL_ERROR(msg) if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_CRITICAL_ERROR, msg)

#define _DEBUG_CALLBACK_CRITICAL_ERROR_RETURN(msg) do {\
if (debug_callback_function) {debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_CRITICAL_ERROR, msg);}\
return PIXELCHAR_FAILED;\
} while(0)

static void (*pixelchar_renderer_backend_deinit_functions[])(struct pixelchar_renderer* pcr) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_renderer_backend_vulkan_deinit,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_renderer_backend_opengl_deinit,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	_pixelchar_renderer_backend_direct_deinit,
#endif
};

static uint32_t(*pixelchar_renderer_backend_add_font_functions[])(struct pixelchar_renderer* pcr, struct pixelfont* font, uint32_t index) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_renderer_backend_vulkan_add_font,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_renderer_backend_opengl_add_font,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	_pixelchar_renderer_backend_direct_add_font,
#endif
};

static void (*pixelchar_renderer_backend_render_functions[])(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_renderer_backend_vulkan_render,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	_pixelchar_renderer_backend_opengl_render,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	_pixelchar_renderer_backend_direct_render,
#endif
};

#endif