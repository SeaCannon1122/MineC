#pragma once

#ifndef PIXELCHAR_FONT_H
#define PIXELCHAR_FONT_H

#include "backend/pixelchar_backend.h"

#define PIXELFONT_MAX_NAME_SIZE 31

struct pixelchar_font
{
	struct
	{
		int a;
#ifdef _PIXELCHAR_BACKEND_VULKAN
		struct pixelchar_font_backend_vulkan vulkan;
#endif

	} backends;

	uint32_t resolution;

	void* memory_handle;

	uint32_t mappings_count;
	uint32_t* mappings;

	uint32_t bitmaps_count;
	uint32_t* widths;
	void* bitmaps;

	uint32_t references;
};

struct pixelchar_font_metadata
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

enum pixelchar_font_resolution
{
	PIXELCHAR_FONT_RESOLUTION_8X8 = 8,
	PIXELCHAR_FONT_RESOLUTION_16X16 = 16,
	PIXELCHAR_FONT_RESOLUTION_32X32 = 32,
	PIXELCHAR_FONT_RESOLUTION_64X64 = 64
};

#define PIXELCHAR_PAD(x) ((x) + 8 - ((x) % 8))

uint32_t pixelchar_font_create(struct pixelchar_font* font, void* font_file_data, size_t font_file_data_size);
uint32_t pixelchar_font_destroy(struct pixelchar_font* font);

#endif