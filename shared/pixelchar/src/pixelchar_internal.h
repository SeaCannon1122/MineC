#pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>
#include <backend/backend.h>

#define PIXELFONT_INVALID 255

#define PIXELCHAR_PAD(x, bytes) ((x) + (bytes) - ((x) % (bytes)))

typedef struct _pixelchar_renderer_char {
	uint32_t bitmapIndex;
	uint32_t flags;
	uint16_t fontIndex;
	uint16_t fontResolution;
	uint16_t scale;
	uint16_t bitmapWidth;
	int32_t position[2];
	uint8_t color[4];
	uint8_t backgroundColor[4];
} _pixelchar_renderer_char;

typedef struct PixelcharFont_T
{
	void* backends[PIXELCHAR_BACKEND_s_COUNT];
	uint32_t backends_reference_count[PIXELCHAR_BACKEND_s_COUNT];

	uint32_t resolution;

	uint32_t mappings_count;
	uint32_t* mappings;

	uint32_t bitmaps_count;
	uint8_t* widths;
	void* bitmaps;

	uint32_t reference_count;

	bool destroyed;
} PixelcharFont_T;

typedef struct PixelcharRenderer_T
{
	void* backends[PIXELCHAR_BACKEND_s_COUNT];

	PixelcharFont fonts[PIXELCHAR_RENDERER_MAX_FONT_COUNT];
	bool font_backends_referenced[PIXELCHAR_RENDERER_MAX_FONT_COUNT][PIXELCHAR_BACKEND_s_COUNT];

	uint32_t queue_total_length;
	uint32_t queue_filled_length;
	Pixelchar queue[];
} PixelcharRenderer_T;

typedef struct _pixelchar_font_metadata
{
	size_t total_size;
	size_t metadata_section_size;
	size_t mappings_section_size;
	size_t widths_section_size;
	size_t bitmaps_section_size;

	uint8_t name[32];
	uint32_t mappings_count;
	uint32_t bitmaps_count;
	uint32_t resolution;
} _pixelchar_font_metadata;

extern PIXELCHAR_DEBUG_CALLBACK_FUNCTION debug_callback_function;

void _pixelchar_renderer_convert_queue(PixelcharRenderer renderer, uint32_t backend_index);

#ifdef __cplusplus
}
#endif

#endif