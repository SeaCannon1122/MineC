#include "pixelchar_internal.h"

uint32_t _pixelchar_font_backend_directx_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index)
{
	if (font->backends.directx.references == 0)
	{

	}
	font->backends.directx.references++;

	return PIXELCHAR_SUCCESS;
}

void _pixelchar_font_backend_directx_reference_subtract(struct pixelchar_font* font)
{
	if (font->backends.directx.references == 1)
	{

	}

	font->backends.directx.references--;
}

void _pixelchar_font_backend_directx_reference_init(struct pixelchar_font* font)
{
	font->backends.directx.references = 0;
}