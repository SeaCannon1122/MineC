#pragma once

#ifndef PIXELCHAR_PIXELCHAR_RENDERER_H
#define PIXELCHAR_PIXELCHAR_RENDERER_H

#include <stdint.h>
#include "backend/pixelchar_renderer_backend.h"

#define PIXELCHAR_MAX_FONTS 4

typedef void (*PIXELCHAR_DEBUG_CALLBACK_FUNCTION)(uint32_t type, uint8_t* message);

enum pixelchar_return_type
{
	PIXELCHAR_SUCCESS = 0,
	PIXELCHAR_FAILED = 1,
};

enum pixelchar_debug_message_type
{
	PIXELCHAR_DEBUG_MESSAGE_TYPE_WARNING,
	PIXELCHAR_DEBUG_MESSAGE_TYPE_ERROR,
	PIXELCHAR_DEBUG_MESSAGE_TYPE_CRITICAL_ERROR,
};

struct pixelchar {
	uint8_t color[4];
	uint8_t background_color[4];
	uint32_t value;
	uint16_t position[2];
	uint16_t masks;
	int16_t size;
};

struct pixelchar_renderer
{

	union
	{
		int a;
#ifdef _PIXELCHAR_BACKEND_VULKAN
		struct pixelchar_vulkan_backend vulkan;
#endif


	} backend;

	int32_t backend_selected;

	uint32_t char_count;

	struct pixelchar* char_buffer;
	uint32_t char_buffer_length;
};

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render);

uint32_t pixelchar_renderer_add_font(struct pixelchar_renderer* pcr, struct pixelfont* font, uint32_t index);

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count);

void pixelchar_renderer_render(struct pixelchar_renderer* pcr, uint32_t width, uint32_t height);

void pixelchar_renderer_backend_deinit(struct pixelchar_renderer* pcr);

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr);

#endif
