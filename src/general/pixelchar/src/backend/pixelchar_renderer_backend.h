#pragma once

#ifndef PIXELCHAR_RENDERER_BACKEND_H
#define PIXELCHAR_RENDERER_BACKEND_H

enum pixelchar_renderer_backend_enum
{
	PIXELCHAR_RENDERER_BACKEND_NONE = -1,
#ifdef _PIXELCHAR_BACKEND_VULKAN
	PIXELCHAR_RENDERER_BACKEND_VULKAN,
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
	PIXELCHAR_RENDERER_BACKEND_OPENGL,
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT
	PIXELCHAR_RENDERER_BACKEND_DIRECT,
#endif
	PIXELCHAR_RENDERER_BACKEND_MAX_ENUM,
};

#ifdef _PIXELCHAR_BACKEND_VULKAN
#include "backend/vulkan/vulkan_backend.h"
#endif
#ifdef _PIXELCHAR_BACKEND_OPENGL
#include "backend/opengl/opengl_backend.h"
#endif
#ifdef _PIXELCHAR_BACKEND_DIRECT 
#include "backend/direct/direct_backend.h"
#endif

#endif
