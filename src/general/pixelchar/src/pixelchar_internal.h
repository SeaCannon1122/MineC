#pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#include <stdlib.h>
#include <string.h>

#include "pixelchar_renderer.h"
#include "pixelchar.h"

extern PIXELCHAR_DEBUG_CALLBACK_FUNCTION debug_callback_function;

struct internal_pixelchar
{
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t bitmap_index;
	uint8_t font;
	uint8_t bitmap_width;
	uint8_t masks;
	uint8_t font_resolution;
	uint16_t position[2][2];
};

#define _DEBUG_CALLBACK_WARNING(msg) do { if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING, msg); } while(0)

#define _DEBUG_CALLBACK_ERROR(msg) do { if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR, msg); } while(0)

#define _DEBUG_CALLBACK_ERROR_RETURN(msg) do {\
if (debug_callback_function) {debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR, msg);}\
return PIXELCHAR_FAILED;\
} while(0)

#define _DEBUG_CALLBACK_CRITICAL_ERROR(msg) do { if (debug_callback_function) debug_callback_function(PIXELCHAR_DEBUG_MESSAGE_TYPE_CRITICAL_ERROR, msg); } while(0)

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

#endif