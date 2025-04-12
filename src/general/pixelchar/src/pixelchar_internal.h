#pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#include "pixelchar_renderer.h"

struct pixelchar {
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t value;
	uint16_t font;
	uint16_t masks;
	uint16_t position[2][2];
};

enum pixelchar_mask {
	PIXELCHAR_MASK_UNDERLINE = 1,
	PIXELCHAR_MASK_CURSIVE = 2,
	PIXELCHAR_MASK_SHADOW = 4,
	PIXELCHAR_MASK_BACKGROUND = 8,
};

enum pixelchar_return_type
{
	PIXELCHAR_SUCCESS = 0,
	PIXELCHAR_FAILED = 1,
};

typedef void (*PIXELCHAR_DEBUG_CALLBACK_FUNCTION)(uint32_t type, uint8_t* message);

enum pixelchar_debug_message_type
{
	PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING,
	PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR,
	PIXELCHAR_DEBUG_MESSAGE_TYPE_CRITICAL_ERROR,
};


uint32_t pixelchar_set_debug_callback(PIXELCHAR_DEBUG_CALLBACK_FUNCTION callback_function);

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

#include <stdlib.h>
#include <string.h>

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