#pragma once

#ifndef PIXELCHAR_PIXELCHAR_RENDERER_H
#define PIXELCHAR_PIXELCHAR_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include "backend/pixelchar_renderer_backend.h"
#include "pixelchar_renderer_max_pixelfonts.h"

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

	struct
	{
		bool init;
		uint32_t resolution;

		void* memory_handle;

		uint32_t* mappings;
		uint32_t mappings_count;

		uint32_t* widths;
		uint32_t widths_count;

	} font_info[PIXELCHAR_RENDERER_MAX_FONTS];

	uint32_t char_count;
	struct internal_pixelchar* char_buffer;
	uint32_t char_buffer_length;
};

uint32_t pixelchar_renderer_create(struct pixelchar_renderer* pcr, uint32_t max_chars_to_render);

uint32_t pixelchar_renderer_add_font(struct pixelchar_renderer* pcr, void* font, size_t size, uint32_t index);

void pixelchar_renderer_queue_pixelchars(struct pixelchar_renderer* pcr, struct pixelchar* chars, uint32_t char_count);

void pixelchar_renderer_backend_deinit(struct pixelchar_renderer* pcr);

void pixelchar_renderer_destroy(struct pixelchar_renderer* pcr);

#endif
