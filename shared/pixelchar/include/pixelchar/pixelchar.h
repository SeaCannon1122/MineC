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
#define PIXELCHAR_RENDERER_MAX_BACKEND_COUNT 16

typedef void (*PIXELCHAR_DEBUG_CALLBACK_FUNCTION)(uint32_t type, uint8_t* message);

typedef enum PixelcharResult
{
	PIXELCHAR_SUCCESS = 0,
	PIXELCHAR_ERROR_OUT_OF_MEMORY,
	PIXELCHAR_ERROR_INVALID_ARGUMENTS,
	PIXELCHAR_ERROR_FULL_QUEUE,
	PIXELCHAR_ERROR_INVALID_FONT_DATA,
	PIXELCHAR_ERROR_BACKEND_SLOT_ALREADY_IN_USED,
	PIXELCHAR_ERROR_BACKEND_SLOT_NOT_IN_USED,
	PIXELCHAR_ERROR_BACKEND_API
} PixelcharResult;

typedef enum PixelcharFlagBits
{
	PIXELCHAR_UNDERLINE_BIT     = 0b1,
	PIXELCHAR_ITALIC_BIT        = 0b10,
	PIXELCHAR_SHADOW_BIT        = 0b100,
	PIXELCHAR_BACKGROUND_BIT    = 0b1000,
	PIXELCHAR_STRIKETHROUGH_BIT = 0b10000,
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

void pixelcharSetDebugCallbackFunction(PIXELCHAR_DEBUG_CALLBACK_FUNCTION pCallbackFunction);

PixelcharResult pixelcharFontCreate(const void* fontData, size_t dataSize, PixelcharFont* pFont);
void pixelcharFontDestroy(PixelcharFont font);

PixelcharResult pixelcharRendererCreate(uint32_t charQueueLength, PixelcharRenderer* pRenderer);
void pixelcharRendererDestroy(PixelcharRenderer renderer);

PixelcharResult pixelcharRendererBindFont(PixelcharRenderer renderer, PixelcharFont font, uint32_t bindingIndex);

PixelcharResult pixelcharRendererEnqueCharacters(PixelcharRenderer renderer, Pixelchar* characters, uint32_t characterCount);
void pixelcharRendererResetQueue(PixelcharRenderer renderer);

uint32_t pixelcharGetCharacterRenderingWidth(PixelcharRenderer renderer, Pixelchar* character);
uint32_t pixelcharGetCharacterRenderingSpacing(PixelcharRenderer renderer, Pixelchar* character0, Pixelchar* character1);

#ifdef __cplusplus
}
#endif

#endif