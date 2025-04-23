#pragma once

#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

enum renderer_backend
{
	RENDERER_BACKEND_VULKAN,
	RENDERER_BACKEND_OPENGL,
	RENDERER_BACKEND_DIRECT
};

#ifdef _MINEC_CLIENT_RENDERER_BACKEND_VULKAN
	#include "vulkan/backend_vulkan.h"
#endif
#ifdef _MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "opengl/backend_opengl.h"
#endif 
#ifdef _MINEC_CLIENT_RENDERER_BACKEND_DIRECT
#include "direct/backend_direct.h"
#endif 

#endif
