#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#define MINEC_CLIENT_ERROR_BACKEND_CRASHED 2

#define RENDERER_MAX_BACKEND_COUNT 8
#define RENDERER_MAX_BACKEND_DEVICE_COUNT 8

struct renderer_backend_info
{
	uint8_t name[64];
	uint8_t description[64];
};

struct renderer_backend_device_info
{
	uint8_t name[64];
	uint8_t description[128];

	bool usable;
	bool disable_vsync_support;
	bool triple_buffering_support;
};

struct renderer_backend_infos
{
	struct renderer_backend_info infos[RENDERER_MAX_BACKEND_COUNT];
	uint32_t count;
};

struct renderer_backend_device_infos
{
	struct renderer_backend_device_info infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];
	uint32_t count;
};

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "opengl/backend_opengl.h"
#define RENDERER_BACKEND_COUNTER_OPENGL 1
#else
struct renderer_backend_opengl { int _empty; };
#define RENDERER_BACKEND_COUNTER_OPENGL 0
#endif

#ifdef MINEC_CLIENT_RENDERER_BACKEND_VULKAN
#include "vulkan/backend_vulkan.h"
#define RENDERER_BACKEND_COUNTER_VULKAN 1
#else
struct renderer_backend_vulkan { int _empty; };
#define RENDERER_BACKEND_COUNTER_VULKAN 0
#endif

struct renderer_backend
{
	uint32_t index;
	union
	{
		struct renderer_backend_opengl opengl;
		struct renderer_backend_vulkan vulkan;
	};
};

void renderer_backend_get_infos(
	struct minec_client* client,
	struct renderer_backend_infos* infos
);

uint32_t renderer_backend_base_create(
	struct minec_client* client, 
	uint32_t backend_index, 
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos
);
void renderer_backend_base_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_device_create(
	struct minec_client* client, 
	uint32_t device_index
);
void renderer_backend_device_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_swapchain_create(
	struct minec_client* client,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering
);
void renderer_backend_swapchain_destroy(
	struct minec_client* client
);

uint32_t renderer_backend_frame_start(
	struct minec_client* client
);

uint32_t renderer_backend_frame_submit(
	struct minec_client* client
);

#endif

#endif