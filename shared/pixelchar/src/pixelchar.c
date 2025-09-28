#include <pixelchar/pixelchar.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LOG_RETURN(message, ...) \
{ \
	if (logCallback)\
	{\
		snprintf(callback_message_buffer, sizeof(callback_message_buffer), message, ##__VA_ARGS__);\
		logCallback(callback_message_buffer, logCallbackUserParameter);\
	}\
	return PIXELCHAR_ERROR_INVALID_FONT_DATA;\
} \

#define PAD(size, pad_size) (((size) + (pad_size) - 1) / (pad_size) * (pad_size))

PixelcharResult pixelcharFontLoadFromFileData(
	const void* fontFileData,
	size_t fontFileDataSize,
	PixelcharFont** ppFont,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParameter
)
{
	PixelcharFontFileHeader* header = fontFileData;

	uint8_t callback_message_buffer[4096];

	if (fontFileDataSize <= sizeof(PixelcharFontFileHeader))
		LOG_RETURN("ERROR: 'fontFileDataSize' (0x%x) <= 'sizeof(PixelcharFontFileHeader)' (0x%x)", fontFileDataSize, sizeof(PixelcharFontFileHeader));

	if (fontFileDataSize < header->headerSectionSize + header->mappingSectionSize + header->bitmapMetadataSectionSize + header->bitmapDataSectionSize)
		LOG_RETURN(
			"ERROR: 'fontFileDataSize' (0x%x) < '"
			"'PixelcharFontFileHeader::headerSectionSize' (0x%x) + "
			"'PixelcharFontFileHeader::mappingSectionSize' (0x%x) + "
			"'PixelcharFontFileHeader::bitmapMetadataSectionSize' (0x%x) + "
			"'PixelcharFontFileHeader::bitmapDataSectionSize' (0x%x)",
			fontFileDataSize,
			header->headerSectionSize,
			header->mappingSectionSize,
			header->bitmapMetadataSectionSize,
			header->bitmapDataSectionSize
		);

	if (header->mappingCount == 0) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::mappingCount' (0x0) is 0x0");
	if (header->mappingSectionSize < header->mappingCount * sizeof(uint32_t)) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::mappingSectionSize' (0x%x) < 'PixelcharFontFileHeader::mappingCount' (0x%x) * 'sizeof(uint32_t)' (0x%x)", header->mappingSectionSize, header->mappingCount, sizeof(uint32_t));
	if (header->bitmapCount == 0) LOG_RETURN("'ERROR: PixelcharFontFileHeader::mappingCount' (0x0) is 0");
	if (header->bitmapMetadataSectionSize < header->bitmapCount * sizeof(PixelcharBitmapMetadata)) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::bitmapMetadataSectionSize' (0x%x) < 'PixelcharFontFileHeader::bitmapCount' (0x%x) * 'sizeof(PixelcharBitmapMetadata)' (0x%x)", header->bitmapMetadataSectionSize, header->bitmapCount, sizeof(PixelcharBitmapMetadata));
	if (header->bitmapDataSectionSize < header->bitmapCount * header->resolution * header->resolution / 8) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::bitmapDataSectionSize' (0x%x) < 'PixelcharFontFileHeader::bitmapCount' (0x%x) * 'PixelcharFontFileHeader::resolution' (0x%x)^2 / 8", header->bitmapDataSectionSize, header->bitmapCount, header->resolution);
	if (header->resolution == 0) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::resolution' (0x0) is 0");
	if (header->resolution % 8) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::resolution' (0x%x) % 8 is not 0", header->resolution);
	if (header->defaultBitmapIndex >= header->bitmapCount) LOG_RETURN("ERROR: 'PixelcharFontFileHeader::defaultBitmapIndex' (0x%x) >= 'PixelcharFontFileHeader::bitmapCount' (0x%x)", header->defaultBitmapIndex, header->bitmapCount);

	uint32_t* mappings = (uintptr_t)header + (uintptr_t)header->headerSectionSize;
	PixelcharBitmapMetadata* metadata = (uintptr_t)mappings + (uintptr_t)header->mappingSectionSize;
	uint32_t* bitmaps = (uintptr_t)metadata + (uintptr_t)header->bitmapMetadataSectionSize;

	for (uint32_t i = 0; i < header->mappingCount; i++) if (mappings[i] >= header->bitmapCount) LOG_RETURN("ERROR: Mapping[0x%x] (0x%x) >= 'PixelcharFontFileHeader::bitmapCount' (0x%x)", mappings[i], header->bitmapCount);

	for (uint32_t i = 0; i < header->bitmapCount; i++)
	{
		if (metadata[i].width == 0) LOG_RETURN("ERROR: PixelcharBitmapMetadata[0x%x]::width (0x0) == 0", i);
		if (metadata[i].width > header->resolution) LOG_RETURN("ERROR: PixelcharBitmapMetadata[0x%x]::width (0x%x) > 'PixelcharFontFileHeader::resolution' (0x%x)", i, metadata[i].width, header->resolution);
		if (metadata[i].thickness == 0) LOG_RETURN("ERROR: PixelcharBitmapMetadata[0x%x]::thickness (0x0) == 0", i);
		if (metadata[i].thickness > header->resolution) LOG_RETURN("ERROR: PixelcharBitmapMetadata[0x%x]::thickness (0x%x) > 'PixelcharFontFileHeader::resolution' (0x%x)", i, metadata[i].thickness, header->resolution);
	}
	
	PixelcharFont* font;

	if ((font = malloc(
		PAD(sizeof(PixelcharFont), 32) +
		PAD(header->mappingCount * sizeof(uint32_t), 32) +
		PAD(header->bitmapCount * sizeof(PixelcharBitmapMetadata), 32) +
		PAD(header->bitmapCount * header->resolution * header->resolution / 8, 32)
	)) == NULL) return PIXELCHAR_ERROR_OUT_OF_MEMORY;
	else
	{
		font->mappingCount = header->mappingCount;
		font->bitmapCount = header->bitmapCount;
		font->resolution = header->resolution;
		font->defaultBitmapIndex = header->defaultBitmapIndex;

		font->pMappings = (uintptr_t)font + (uintptr_t)PAD(sizeof(PixelcharFont), 32);
		font->pBitmapMetadata = (uintptr_t)font->pMappings + (uintptr_t)PAD(header->mappingCount * sizeof(uint32_t), 32);
		font->pBitmaps = (uintptr_t)font->pBitmapMetadata + (uintptr_t)PAD(header->bitmapCount * sizeof(PixelcharBitmapMetadata), 32);

		memcpy(font->pMappings, mappings, header->mappingCount * sizeof(uint32_t));
		memcpy(font->pBitmapMetadata, metadata, header->bitmapCount * sizeof(PixelcharBitmapMetadata));
		memcpy(font->pBitmaps, bitmaps, header->bitmapCount * header->resolution * header->resolution / 8);
	}

	*ppFont = font;
	return PIXELCHAR_SUCCESS;
}

void pixelcharFill(int32_t posX, int32_t posY, uint32_t character, uint32_t scale, uint32_t modifiers, uint32_t color, uint32_t backgroundColor, PixelcharFont* pFont, Pixelchar* pChar)
{
	pChar->position[0] = posX;
	pChar->position[1] = posY;
	pChar->color = color;
	pChar->backgroundColor = backgroundColor;
	pChar->scale = scale;
	pChar->modifiers = modifiers;

	if (character < pFont->mappingCount) pChar->bitmapIndex = pFont->pMappings[character];
	else pChar->bitmapIndex = pFont->defaultBitmapIndex;

	pChar->bitmapWidth = pFont->pBitmapMetadata[pChar->bitmapIndex].width;
	pChar->bitmapThickness = pFont->pBitmapMetadata[pChar->bitmapIndex].thickness;

	pChar->width = scale * 8 * (modifiers & PIXELCHAR_MODIFIER_BOLD_BIT ? pChar->bitmapWidth + pChar->bitmapThickness : pChar->bitmapWidth) / pFont->resolution + scale;
}