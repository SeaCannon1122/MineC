#pragma once

#ifndef PIXELCHAR_PIXELCHAR_RENDERER_H
#define PIXELCHAR_PIXELCHAR_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include "pixelchar_font.h"
#include "pixelchar_renderer_max_pixelfonts.h"

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
	struct internal_pixelchar* char_buffer;
	uint32_t char_buffer_length;

	struct pixelchar_font* fonts[PIXELCHAR_RENDERER_MAX_FONTS];
};

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render);

void pixelchar_renderer_set_font(struct pixelchar_renderer* pcr, struct pixelchar_font* font, uint32_t index);

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count);

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr);

#endif
