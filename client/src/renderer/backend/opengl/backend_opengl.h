#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_OPENGL_H

#include "../backend_internal.h"

static void renderer_backend_opengl_log(struct minec_client* client, uint8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	minec_client_log_v(client, "[RENDERER][OPENGL]", message, args);
	va_end(args);
}

#define VULKAN_SWAPCHAIN_IMAGE_COUNT 2

struct renderer_backend_opengl_base
{
	int a;
};

struct renderer_backend_vulkan_device
{
	int a;
};

struct renderer_backend_vulkan_pipelines_resources
{
	struct
	{
		bool usable;
	} pixelchar_renderer;
};

#endif
