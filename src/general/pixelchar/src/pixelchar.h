#pragma once 

#ifndef PIXELCHAR_RENDERER_H

#endif


#ifndef PIXELCHAR_H
#define PIXELCHAR_H

#define _PIXELCHAR_INTERNAL_EXCLUDE

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

#define PIXELFONT_MAX_NAME_SIZE 31

struct pixelfont_metadata
{
	size_t total_size;
	size_t metadata_section_size;
	size_t mappings_section_size;
	size_t widths_section_size;
	size_t bitmaps_section_size;

	uint8_t name[PIXELFONT_MAX_NAME_SIZE + 1];
	uint32_t mappings_count;
	uint32_t bitmaps_count;
	uint32_t resolution;
};



enum pixelfont_resolution
{
	PIXELFONT_RESOLUTION_8X8 = 8,
	PIXELFONT_RESOLUTION_16X16 = 16,
	PIXELFONT_RESOLUTION_32X32 = 32,
	PIXELFONT_RESOLUTION_64X64 = 64
};

#define PIXELCHAR_PAD(x) ((x) + 8 - ((x) % 8))

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

#endif