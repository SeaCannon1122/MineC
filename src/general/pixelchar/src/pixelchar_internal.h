#pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#include "pixelchar_renderer.h"

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

#include <stdlib.h>
#include <string.h>

#define PIXELFONT_INVALID 255

extern PIXELCHAR_DEBUG_CALLBACK_FUNCTION debug_callback_function;

struct internal_pixelchar {
	uint32_t bitmap_index;
	uint16_t masks;
	uint8_t font;
	uint8_t font_resolution;
	uint8_t scale;
	uint8_t bitmap_width;
	int16_t position[2];
	uint8_t color[4];
	uint8_t background_color[4];
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

static void (*pixelchar_renderer_backend_deinit_functions[])(struct pixelchar_renderer*) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	pixelchar_renderer_backend_vulkan_deinit,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	NULL,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	NULL,
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
	NULL,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	NULL,
#else
	NULL,
#endif
};

static uint32_t (*_pixelchar_font_backend_reference_add_functions[])(struct pixelchar_font*, struct pixelchar_renderer*, uint32_t) = {
#ifdef _PIXELCHAR_BACKEND_VULKAN
	_pixelchar_font_backend_vulkan_reference_add,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	NULL,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	NULL,
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
	NULL,
#else
	NULL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	NULL,
#else
	NULL,
#endif
};

#endif

#endif