#pragma once 

#ifndef PIXELCHAR_H
#define PIXELCHAR_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PIXELCHAR_RENDERER_MAX_FONT_COUNT 8
#define PIXELCHAR_FONT_NAME_BUFFER_SIZE 32

typedef enum PixelcharResult
{
	PIXELCHAR_SUCCESS								= 0x0000,
	PIXELCHAR_INFO_MASK								= 0x0100,
	PIXELCHAR_INFO_FULL_QUEUE						= 0x0101,
	PIXELCHAR_ERROR_MASK							= 0x0200,
	PIXELCHAR_ERROR_OUT_OF_MEMORY					= 0x0201,
	PIXELCHAR_ERROR_INVALID_ARGUMENTS				= 0x0202,
	PIXELCHAR_ERROR_INVALID_FONT_DATA				= 0x0203
} PixelcharResult;

typedef enum PixelcharFlagBits
{
	PIXELCHAR_UNDERLINE_BIT     =      0b1,
	PIXELCHAR_ITALIC_BIT        =     0b10,
	PIXELCHAR_SHADOW_BIT        =    0b100,
	PIXELCHAR_BACKGROUND_BIT    =   0b1000,
	PIXELCHAR_STRIKETHROUGH_BIT =  0b10000,
	PIXELCHAR_BOLD_BIT          = 0b100000,
} PixelcharFlagBits;

typedef struct Pixelchar
{
	uint32_t character;
	uint32_t flags;
	uint32_t fontIndex;
	uint32_t scale;
	int32_t position[2];
	uint8_t color[4];
	uint8_t backgroundColor[4];
} Pixelchar;

typedef struct PixelcharFont_T* PixelcharFont;

const uint8_t* pixelcharGetResultAsString(PixelcharResult result);

PixelcharResult pixelcharFontCreate(const void* fontData, size_t dataSize, PixelcharFont* pFont);
void pixelcharFontDestroy(PixelcharFont font);
void pixelcharFontGetName(PixelcharFont font, uint8_t* buffer);

uint32_t pixelcharGetCharacterRenderingWidth(Pixelchar* character, PixelcharFont* fonts);
uint32_t pixelcharGetCharacterRenderingSpacing(Pixelchar* character0, Pixelchar* character1, PixelcharFont* fonts);


#endif