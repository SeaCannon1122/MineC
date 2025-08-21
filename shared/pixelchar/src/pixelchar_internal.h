  #pragma once

#ifndef PIXLECHAR_INTERNAL_H
#define PIXELCHAR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

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
	uint8_t name[PIXELCHAR_FONT_NAME_BUFFER_SIZE];

	uint32_t resolution;

	uint32_t mappings_count;
	uint32_t* mappings;

	uint32_t bitmaps_count;
	uint8_t* widths;
	void* bitmaps;

	uint32_t reference_count;
	bool destroyed;
} PixelcharFont_T;

typedef struct _pixelchar_font_metadata
{
	size_t total_size;
	size_t metadata_section_size;
	size_t mappings_section_size;
	size_t widths_section_size;
	size_t bitmaps_section_size;

	uint8_t name[PIXELCHAR_FONT_NAME_BUFFER_SIZE];
	uint32_t mappings_count;
	uint32_t bitmaps_count;
	uint32_t resolution;
} _pixelchar_font_metadata;

void _convert_chars_to_render_chars(Pixelchar* chars, uint32_t char_count, PixelcharFont* fonts);

#ifdef __cplusplus
}
#endif

#endif