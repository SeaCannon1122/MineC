#include "pixelchar_internal.h"

uint32_t _pixelchar_font_backend_opengl_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index)
{
	if (font->backends.opengl.references == 0)
	{
		glGenBuffers(1, &font->backends.opengl.buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, font->backends.opengl.buffer);

		glBufferData(GL_SHADER_STORAGE_BUFFER, font->bitmaps_count * font->resolution * font->resolution / 8, font->bitmaps, GL_STATIC_DRAW);
	}

	font->backends.opengl.references++;

	return PIXELCHAR_SUCCESS;
}

void _pixelchar_font_backend_opengl_reference_subtract(struct pixelchar_font* font)
{
	if (font->backends.opengl.references == 1)
	{
		glDeleteBuffers(1, &font->backends.opengl.buffer);
	}

	font->backends.opengl.references--;
}

void _pixelchar_font_backend_opengl_reference_init(struct pixelchar_font* font)
{
	font->backends.opengl.references = 0;
}