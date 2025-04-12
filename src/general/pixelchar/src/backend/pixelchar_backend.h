#pragma once

#ifndef PIXELCHAR_BACKEND_H
#define PIXELCHAR_BACKEND_H

enum pixelchar_backend_bit_enum
{
	PIXELCHAR_BACKEND_VULKAN_BIT = 1,
	PIXELCHAR_BACKEND_OPENGL_BIT = 2,
	PIXELCHAR_BACKEND_DIRECT_BIT = 4,
	PIXELCHAR_BACKEND_MAX_BIT = 8
};

#ifdef _PIXELCHAR_BACKEND_VULKAN
#include "vulkan/pixelchar_renderer_backend_vulkan.h"
#include "vulkan/pixelchar_font_backend_vulkan.h"
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
#include "opengl/pixelchar_renderer_backend_opengl.h"
#include "opengl/pixelchar_font_backend_opengl.h"
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
#include "direct/pixelchar_renderer_backend_direct.h"
#include "direct/pixelchar_font_backend_direct.h"
#endif

#endif
