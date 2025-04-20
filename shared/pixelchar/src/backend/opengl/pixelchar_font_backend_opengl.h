#pragma once

#ifndef PIXELCHAR_FONT_BACKEND_OPENGL_H
#define PIXELCHAR_FONT_BACKEND_OPENGL_H

#include <stdint.h>
#include <glad/glad.h>

struct pixelchar_font_backend_opengl
{
	uint32_t references;

	GLuint buffer;
};

struct pixelchar_font;

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

uint32_t _pixelchar_font_backend_opengl_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index);

void _pixelchar_font_backend_opengl_reference_subtract(struct pixelchar_font* font);

void _pixelchar_font_backend_opengl_reference_init(struct pixelchar_font* font);

#endif

#endif
