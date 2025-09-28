#pragma once 

#ifndef LIB_PIXELCHAR_H
#define LIB_PIXELCHAR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//Library

typedef enum PixelcharResult
{
	PIXELCHAR_SUCCESS = 0x0000,
	PIXELCHAR_ERROR_MASK = 0x0200,
	PIXELCHAR_ERROR_OUT_OF_MEMORY = 0x0201,
	PIXELCHAR_ERROR_INVALID_ARGUMENTS = 0x0202,
	PIXELCHAR_ERROR_INVALID_FONT_DATA = 0x0203
} PixelcharResult;

//Pixelfont

#define PIXELCHAR_FONT_NAME_BUFFER_SIZE 32

typedef struct PixelcharBitmapMetadata
{
	uint32_t width;
	uint32_t thickness;
} PixelcharBitmapMetadata;

typedef struct PixelcharFontFileHeader
{
	uint64_t headerSectionSize;
	uint64_t mappingSectionSize;
	uint64_t bitmapMetadataSectionSize;
	uint64_t bitmapDataSectionSize;

	uint8_t name[PIXELCHAR_FONT_NAME_BUFFER_SIZE];
	uint32_t mappingCount;
	uint32_t defaultBitmapIndex;
	uint32_t resolution;
	uint32_t bitmapCount;
} PixelcharFontFileHeader;

typedef struct PixelcharFont
{
	uint32_t resolution;

	uint32_t mappingCount;
	uint32_t* pMappings;
	uint32_t defaultBitmapIndex;

	uint32_t bitmapCount;
	PixelcharBitmapMetadata* pBitmapMetadata;
	uint32_t* pBitmaps;
} PixelcharFont;

typedef void (*PixelcharLogCallback)(uint8_t* message, void* userParameter);

//Pixelchar

typedef enum PixelcharModifierBits
{
	PIXELCHAR_MODIFIER_BOLD_BIT = 1,
	PIXELCHAR_MODIFIER_ITALIC_BIT = 2,
	PIXELCHAR_MODIFIER_UNDERLINE_BIT = 4,
	PIXELCHAR_MODIFIER_STRIKETHROUGH_BIT = 8,
	PIXELCHAR_MODIFIER_SHADOW_BIT = 16,
	PIXELCHAR_MODIFIER_BACKGROUND_BIT = 32,
} PixelcharModifierBits;


typedef struct Pixelchar
{
	int32_t position[2];
	uint32_t width;
	uint32_t scale;
	uint32_t bitmapIndex;
	uint32_t bitmapWidth;
	uint32_t bitmapThickness;
	uint32_t modifiers;
	uint32_t color;
	uint32_t backgroundColor;
} Pixelchar;

PixelcharResult pixelcharFontLoadFromFileData(
	const void* fontFileData, 
	size_t fontFileDataSize, 
	PixelcharFont** ppFont, 
	PixelcharLogCallback logCallback,
	void* logCallbackUserParameter
);

void pixelcharFill(int32_t posX, int32_t posY, uint32_t character, uint32_t scale, uint32_t modifiers, uint32_t color, uint32_t backgroundColor, PixelcharFont* pFont, Pixelchar* pChar);

#endif