#include <pixelchar_internal.h>

#include <stdlib.h>
#include <string.h>

const uint8_t* pixelcharGetResultAsString(PixelcharResult result)
{
	switch (result)
	{
	case PIXELCHAR_SUCCESS:								return "PIXELCHAR_SUCCESS";
	case PIXELCHAR_INFO_FULL_QUEUE:						return "PIXELCHAR_INFO_FULL_QUEUE";
	case PIXELCHAR_ERROR_OUT_OF_MEMORY:					return "PIXELCHAR_ERROR_OUT_OF_MEMORY";
	case PIXELCHAR_ERROR_INVALID_ARGUMENTS:				return "PIXELCHAR_ERROR_INVALID_ARGUMENTS";
	case PIXELCHAR_ERROR_INVALID_FONT_DATA:				return "PIXELCHAR_ERROR_INVALID_FONT_DATA";
	case PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED:	return "PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED";
	case PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED:		return "PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED";
	case PIXELCHAR_ERROR_BACKEND_API:					return "PIXELCHAR_ERROR_BACKEND_API";
	}

	return "PIXELCHAR_ERROR_UNKNOWN";
}

PixelcharResult pixelcharFontCreate(const void* fontData, size_t dataSize, PixelcharFont* pFont)
{
	if (fontData == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (dataSize < sizeof(_pixelchar_font_metadata)) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (pFont == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	
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
	if (font == NULL) return;

	font->destroyed = true;

	if (font->reference_count == 0) free(font);
}

void pixelcharFontGetName(PixelcharFont font, uint8_t* buffer)
{
	memcpy(buffer, font->name, sizeof(font->name));
}

void _pixelchar_renderer_convert_queue(PixelcharRenderer renderer, uint32_t backend_index)
{
	_pixelchar_renderer_char* chars = (_pixelchar_renderer_char*) &renderer->queue;

	for (uint32_t i = 0; i < renderer->queue_filled_length; i++)
	{
		uint32_t character = renderer->queue[i].character;
		if (chars[i].fontIndex >= PIXELCHAR_RENDERER_MAX_FONT_COUNT)
		{
			chars[i].fontIndex = PIXELFONT_INVALID;
			chars[i].bitmapWidth = 8;
			chars[i].fontResolution = 8;
		}
		else if (renderer->fonts[chars[i].fontIndex] == NULL || renderer->font_backends_referenced[chars[i].fontIndex][backend_index] == false)
		{
			chars[i].fontIndex = PIXELFONT_INVALID;
			chars[i].bitmapWidth = 8;
			chars[i].fontResolution = 8;
		}
		else
		{
			if (character >= renderer->fonts[chars[i].fontIndex]->mappings_count)
				chars[i].bitmapIndex = 0;
			else
				chars[i].bitmapIndex = renderer->fonts[chars[i].fontIndex]->mappings[character];

			chars[i].bitmapWidth = renderer->fonts[chars[i].fontIndex]->widths[chars[i].bitmapIndex];
			chars[i].fontResolution = renderer->fonts[chars[i].fontIndex]->resolution;
		}
	}
}

PixelcharResult pixelcharRendererCreate(uint32_t charQueueLength, PixelcharRenderer* pRenderer)
{
	if (charQueueLength == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (pRenderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	PixelcharRenderer pcr = malloc(sizeof(PixelcharRenderer_T) + sizeof(Pixelchar) * charQueueLength);
	if (pcr == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;

	memset(pcr, 0, sizeof(PixelcharRenderer_T));

	pcr->queue_total_length = charQueueLength;

	*pRenderer = pcr;
	return PIXELCHAR_SUCCESS;
}

void pixelcharRendererDestroy(PixelcharRenderer renderer)
{
	if (renderer == NULL) return;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->fonts[i] != NULL) pixelcharRendererBindFont(renderer, NULL, i);
	}

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_BACKEND_COUNT; i++)
	{
		if (renderer->backends[i].data != NULL) renderer->backends[i].deinitialize_function(renderer, i);
	}

	free(renderer);
}

void pixelcharRendererHardResetBackendSlot(PixelcharRenderer renderer, uint32_t backendSlotIndex)
{
	if (renderer == NULL) return;
	if (backendSlotIndex >= PIXELCHAR_RENDERER_MAX_BACKEND_COUNT) return;

	free(renderer->backends[backendSlotIndex].data);
	renderer->backends[backendSlotIndex].data = NULL;

	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (renderer->font_backends_referenced[i][backendSlotIndex])
		{
			renderer->fonts[i]->backends_reference_count[backendSlotIndex]--;
			if (renderer->fonts[i]->backends_reference_count[backendSlotIndex] == 0) free(renderer->fonts[i]->backends[backendSlotIndex]);
			renderer->font_backends_referenced[i][backendSlotIndex] = false;
		}
	}
}

PixelcharResult pixelcharRendererBindFont(PixelcharRenderer renderer, PixelcharFont font, uint32_t bindingIndex)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (bindingIndex >= PIXELCHAR_RENDERER_MAX_FONT_COUNT) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	if (renderer->fonts[bindingIndex] != NULL)
	{
		for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_BACKEND_COUNT; i++)
		{
			if (renderer->font_backends_referenced[bindingIndex][i] == true) 
				renderer->backends[i].font_backend_sub_reference_function(renderer, bindingIndex, i);

			renderer->font_backends_referenced[bindingIndex][i] = false;
		}
		renderer->fonts[bindingIndex]->reference_count--;

		if (renderer->fonts[bindingIndex]->reference_count == 0 && renderer->fonts[bindingIndex]->destroyed == true) free(renderer->fonts[bindingIndex]);
	}

	renderer->fonts[bindingIndex] = font;

	if (renderer->fonts[bindingIndex] != NULL)
	{
		for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_BACKEND_COUNT; i++)
		{
			if (renderer->backends[i].data != NULL)
			{
				if (renderer->backends[i].font_backend_add_reference_function(renderer, bindingIndex, i) == PIXELCHAR_SUCCESS)
					renderer->font_backends_referenced[bindingIndex][i] = true;
			}

		}
		renderer->fonts[bindingIndex]->reference_count++;
	}

	return PIXELCHAR_SUCCESS;
}

PixelcharResult pixelcharRendererEnqueCharacters(PixelcharRenderer renderer, Pixelchar* characters, uint32_t characterCount)
{
	if (renderer == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (characters == NULL) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;
	if (characterCount == 0) return PIXELCHAR_ERROR_INVALID_ARGUMENTS;

	if (renderer->queue_total_length - renderer->queue_filled_length == 0) return PIXELCHAR_INFO_FULL_QUEUE;

	uint32_t chars_to_copy_count =
		(
			renderer->queue_total_length - renderer->queue_filled_length < characterCount ?
			renderer->queue_total_length - renderer->queue_filled_length :
			characterCount
			);

	memcpy(renderer->queue, characters, chars_to_copy_count * sizeof(Pixelchar));
	renderer->queue_filled_length += chars_to_copy_count;

	if (chars_to_copy_count == characterCount) return PIXELCHAR_SUCCESS;
	else return PIXELCHAR_INFO_FULL_QUEUE;
}

void pixelcharRendererResetQueue(PixelcharRenderer renderer)
{
	renderer->queue_filled_length = 0;
}

uint32_t pixelcharGetCharacterRenderingWidth(PixelcharRenderer renderer, Pixelchar* character)
{
	if (renderer == NULL) return 0;
	if (character == NULL) return 0;

	if (character->fontIndex >= PIXELCHAR_RENDERER_MAX_FONT_COUNT) return (int32_t)character->scale * 8;
	if (renderer->fonts[character->fontIndex] == NULL) return (int32_t)character->scale * 8;

	uint32_t bitmap_index =
		(
			character->character >= renderer->fonts[character->fontIndex]->mappings_count ?
			0 :
			renderer->fonts[character->fontIndex]->mappings[character->character]
			);

	uint32_t width =
		(
			(int32_t)renderer->fonts[character->fontIndex]->widths[bitmap_index] * (int32_t)character->scale * 8 +
			(int32_t)renderer->fonts[character->fontIndex]->resolution
			- 1
			) /
		(int32_t)renderer->fonts[character->fontIndex]->resolution;

	return width;
}

uint32_t pixelcharGetCharacterRenderingSpacing(PixelcharRenderer renderer, Pixelchar* character0, Pixelchar* character1)
{
	if (renderer == NULL) return 0;
	if (character0 == NULL) return 0;
	if (character1 == NULL) return 0;

	return ((int32_t)character0->scale * 8 + 15) / 16 + ((int32_t)character1->scale * 8) / 16;
}