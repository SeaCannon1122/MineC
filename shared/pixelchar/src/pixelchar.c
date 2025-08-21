#include <pixelchar_internal.h>

#include <stdlib.h>
#include <string.h>

const uint8_t* pixelcharGetResultAsString(PixelcharResult result)
{
	switch (result)
	{
	case PIXELCHAR_SUCCESS:								return (const uint8_t*)"PIXELCHAR_SUCCESS";
	case PIXELCHAR_INFO_FULL_QUEUE:						return (const uint8_t*)"PIXELCHAR_INFO_FULL_QUEUE";
	case PIXELCHAR_ERROR_OUT_OF_MEMORY:					return (const uint8_t*)"PIXELCHAR_ERROR_OUT_OF_MEMORY";
	case PIXELCHAR_ERROR_INVALID_ARGUMENTS:				return (const uint8_t*)"PIXELCHAR_ERROR_INVALID_ARGUMENTS";
	case PIXELCHAR_ERROR_INVALID_FONT_DATA:				return (const uint8_t*)"PIXELCHAR_ERROR_INVALID_FONT_DATA";
	}

	return (const uint8_t*)"PIXELCHAR_ERROR_UNKNOWN";
}

PixelcharResult pixelcharFontCreate(const void* fontData, size_t dataSize, PixelcharFont* pFont)
{
	if (dataSize < sizeof(_pixelchar_font_metadata)) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	
	const _pixelchar_font_metadata* metadata = (const _pixelchar_font_metadata*)fontData;

	size_t bitmap_size = metadata->resolution * metadata->resolution / 8;

	PixelcharFont font = malloc(
		(size_t)PIXELCHAR_PAD(sizeof(PixelcharFont_T), 8)                     +
		(size_t)PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t), 8) + 
		(size_t)PIXELCHAR_PAD(metadata->bitmaps_count * sizeof(uint8_t), 8)   +
		(size_t)PIXELCHAR_PAD(metadata->bitmaps_count * bitmap_size, 8)
	);
	if (font == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

	memset(font, 0, sizeof(PixelcharFont_T));

	font->resolution = metadata->resolution;

	font->mappings_count = metadata->mappings_count;
	font->mappings = (uint32_t*)(
		(size_t)font + 
		(size_t)PIXELCHAR_PAD(sizeof(PixelcharFont_T), 8)
	);
	memcpy(
		font->mappings, 
		(uint32_t*)((size_t)fontData + metadata->metadata_section_size), 
		metadata->mappings_count * sizeof(uint32_t)
	);

	font->bitmaps_count = metadata->bitmaps_count;
	font->widths = (uint8_t*)(
		(size_t)font + 
		(size_t)PIXELCHAR_PAD(sizeof(PixelcharFont_T), 8) + 
		(size_t)PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t), 8)
	);
	memcpy(
		font->widths, 
		(uint8_t*)((size_t)fontData + metadata->metadata_section_size + metadata->mappings_section_size),
		metadata->bitmaps_count * sizeof(uint8_t)
	);
	font->bitmaps = (void*)(
		(size_t)font + 
		(size_t)PIXELCHAR_PAD(sizeof(PixelcharFont_T), 8) + 
		(size_t)PIXELCHAR_PAD(metadata->mappings_count * sizeof(uint32_t), 8) + 
		(size_t)PIXELCHAR_PAD(metadata->bitmaps_count * sizeof(uint8_t), 8)
	);
	memcpy(
		font->bitmaps,
		(void*)((size_t)fontData + metadata->metadata_section_size + metadata->mappings_section_size + metadata->widths_section_size), 
		metadata->bitmaps_count * bitmap_size
	);

	memcpy(font->name, metadata->name, sizeof(font->name));

	*pFont = font;
	return PIXELCHAR_SUCCESS;
}

void pixelcharFontDestroy(PixelcharFont font)
{
	font->destroyed = true;
	if (font->reference_count == 0) free(font);
}

void pixelcharFontGetName(PixelcharFont font, uint8_t* buffer)
{
	memcpy(buffer, font->name, sizeof(font->name));
}

void _convert_chars_to_render_chars(Pixelchar* chars, uint32_t char_count, PixelcharFont* fonts)
{
	_pixelchar_renderer_char* r_chars = chars;

	for (uint32_t i = 0; i < char_count; i++)
	{
		if (chars[i].scale == 0) chars[i].scale = 1;

		if (chars[i].fontIndex >= PIXELCHAR_RENDERER_MAX_FONT_COUNT)
		{
			r_chars[i].fontIndex = PIXELFONT_INVALID;
			r_chars[i].bitmapWidth = 8;
			r_chars[i].fontResolution = 8;
		}
		else if (fonts[chars[i].fontIndex] == NULL)
		{
			r_chars[i].fontIndex = PIXELFONT_INVALID;
			r_chars[i].bitmapWidth = 8;
			r_chars[i].fontResolution = 8;
		}
		else
		{
			if (chars[i].character >= fonts[chars[i].fontIndex]->mappings_count)
				r_chars[i].bitmapIndex = 0;
			else
				r_chars[i].bitmapIndex = fonts[r_chars[i].fontIndex]->mappings[chars[i].character];

			r_chars[i].bitmapWidth = fonts[r_chars[i].fontIndex]->widths[r_chars[i].bitmapIndex];
			r_chars[i].fontResolution = (uint16_t)fonts[r_chars[i].fontIndex]->resolution;
		}
	}
}

uint32_t pixelcharGetCharacterRenderingWidth(Pixelchar* character, PixelcharFont* fonts)
{
	uint32_t scale = (character->scale > 0 ? character->scale : 1);

	if (character->fontIndex >= PIXELCHAR_RENDERER_MAX_FONT_COUNT) return scale * 8;
	if (fonts[character->fontIndex] == NULL) return scale * 8;

	uint32_t bitmap_index =
		(
			character->character >= fonts[character->fontIndex]->mappings_count ?
			0 :
			fonts[character->fontIndex]->mappings[character->character]
		);

	uint32_t width =
		(
			(uint32_t)fonts[character->fontIndex]->widths[bitmap_index] * scale * 8 +
			fonts[character->fontIndex]->resolution
			- 1
		) / fonts[character->fontIndex]->resolution;

	return width;
}

uint32_t pixelcharGetCharacterRenderingSpacing(Pixelchar* character0, Pixelchar* character1, PixelcharFont* fonts)
{
	uint32_t scale0 = (character0->scale > 0 ? character0->scale : 1);
	uint32_t scale1 = (character1->scale > 0 ? character1->scale : 1);

	return (scale0 * 8 + 15) / 16 + (scale1 * 8) / 16;
}