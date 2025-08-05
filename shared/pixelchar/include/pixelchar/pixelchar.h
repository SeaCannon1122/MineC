#pragma once 

#ifndef PIXELCHAR_H
#define PIXELCHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PIXELCHAR_RENDERER_MAX_FONT_COUNT 8
#define PIXELCHAR_RENDERER_MAX_BACKEND_COUNT 32
#define PIXELCHAR_FONT_NAME_BUFFER_SIZE 32

typedef enum PixelcharResult
{
	PIXELCHAR_SUCCESS								= 0x0000,
	PIXELCHAR_INFO_MASK								= 0x0100,
	PIXELCHAR_INFO_FULL_QUEUE						= 0x0101,
	PIXELCHAR_ERROR_MASK							= 0x0200,
	PIXELCHAR_ERROR_OUT_OF_MEMORY					= 0x0201,
	PIXELCHAR_ERROR_INVALID_ARGUMENTS				= 0x0202,
	PIXELCHAR_ERROR_INVALID_FONT_DATA				= 0x0203,
	PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED	= 0x0204,
	PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED		= 0x0205,
	PIXELCHAR_ERROR_BACKEND_API						= 0x0206
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

typedef struct PixelcharRenderer_T* PixelcharRenderer;
typedef struct PixelcharFont_T* PixelcharFont;

const uint8_t* pixelcharGetResultAsString(PixelcharResult result);

PixelcharResult pixelcharFontCreate(const void* fontData, size_t dataSize, PixelcharFont* pFont);
void pixelcharFontDestroy(PixelcharFont font);
void pixelcharFontGetName(PixelcharFont font, uint8_t* buffer);

PixelcharResult pixelcharRendererCreate(uint32_t charQueueLength, PixelcharRenderer* pRenderer);
void pixelcharRendererDestroy(PixelcharRenderer renderer);

void pixelcharRendererHardResetBackendSlot(PixelcharRenderer renderer, uint32_t backendSlotIndex);

PixelcharResult pixelcharRendererBindFont(PixelcharRenderer renderer, PixelcharFont font, uint32_t bindingIndex);

PixelcharResult pixelcharRendererEnqueCharacters(PixelcharRenderer renderer, Pixelchar* characters, uint32_t characterCount);
void pixelcharRendererResetQueue(PixelcharRenderer renderer);

uint32_t pixelcharGetCharacterRenderingWidth(PixelcharRenderer renderer, Pixelchar* character);
uint32_t pixelcharGetCharacterRenderingSpacing(PixelcharRenderer renderer, Pixelchar* character0, Pixelchar* character1);

#ifdef __cplusplus
}
#endif

#endif