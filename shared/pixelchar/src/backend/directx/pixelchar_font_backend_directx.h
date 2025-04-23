#pragma once

#ifndef PIXELCHAR_FONT_BACKEND_DIRECTX_H
#define PIXELCHAR_FONT_BACKEND_DIRECTX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct pixelchar_font_backend_directx
{
	uint32_t references;
};

struct pixelchar_font;

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

uint32_t _pixelchar_font_backend_directx_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index);

void _pixelchar_font_backend_directx_reference_subtract(struct pixelchar_font* font);

void _pixelchar_font_backend_directx_reference_init(struct pixelchar_font* font);

#endif

#ifdef __cplusplus
}
#endif

#endif