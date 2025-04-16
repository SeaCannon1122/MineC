#pragma once

#ifndef PIXELCHAR_RENDERER_H
#define PIXELCHAR_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include "pixelchar_font.h"
#include "pixelchar_renderer_max_pixelfonts.h"

struct pixelchar {
	uint32_t value;
	uint16_t masks;
	uint16_t font;
	uint16_t scale;
	int16_t position[2];
	uint8_t color[4];
	uint8_t background_color[4];
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

struct pixelchar_renderer
{
	struct
	{
		int a;
#ifdef _PIXELCHAR_BACKEND_VULKAN
		struct pixelchar_renderer_backend_vulkan vulkan;
#endif

	} backends;

	uint32_t backends_initialized;

	uint32_t char_count;
	struct pixelchar* char_buffer;
	uint32_t char_buffer_length;

	struct pixelchar_font* fonts[PIXELCHAR_RENDERER_MAX_FONTS];
};

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render);

void pixelchar_renderer_set_font(struct pixelchar_renderer* pcr, struct pixelchar_font* font, uint32_t index);

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count);

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr);

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

void _pixelchar_renderer_render_convert_to_internal_characters(struct pixelchar_renderer* pcr);

#endif // !_PIXELCHAR_INTERNEL_EXCLUDE


#endif
