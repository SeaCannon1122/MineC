#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#define MINEC_CLIENT_ERROR_BACKEND_CRASHED 2

#define DEFINE_RENDERER_BACKEND_DEFAULT_STRUCTURES(name) \
	struct renderer_backend_##name##_base {int _empty;}; \
	struct renderer_backend_##name##_device {int _empty;}; \
	struct renderer_backend_##name##_swapchain {int _empty;}; \

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "opengl/backend_opengl.h"
#define RENDERER_BACKEND_COUNTER_OPENGL 1
#else
DEFINE_RENDERER_BACKEND_DEFAULT_STRUCTURES(opengl)
#define RENDERER_BACKEND_COUNTER_OPENGL 0
#endif

#ifdef MINEC_CLIENT_RENDERER_BACKEND_VULKAN
#include "vulkan/backend_vulkan.h"
#define RENDERER_BACKEND_COUNTER_VULKAN 1
#else
DEFINE_RENDERER_BACKEND_DEFAULT_STRUCTURES(vulkan)
#define RENDERER_BACKEND_COUNTER_VULKAN 0
#endif

#define RENDERER_BACKEND_COUNT RENDERER_BACKEND_COUNTER_OPENGL + RENDERER_BACKEND_COUNTER_VULKAN

#define RENDERER_BACKEND_STRUCTURE(structure, name) struct renderer_backend_##name##_##structure name;

#define RENDERER_BACKEND_STRUCTURES(structure) \
	union { \
		RENDERER_BACKEND_STRUCTURE(structure, opengl)\
		RENDERER_BACKEND_STRUCTURE(structure, vulkan)\
	} \

#define RENDRER_MAX_BACKEND_COUNT 8
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
	struct renderer_backend_info infos[RENDRER_MAX_BACKEND_COUNT];
	uint32_t count;
};

struct renderer_backend_device_infos
{
	struct renderer_backend_device_info infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];
	uint32_t count;
};

struct renderer_backend_base
{
	uint32_t index;
	RENDERER_BACKEND_STRUCTURES(base);
};

struct renderer_backend_device
{
	struct renderer_backend_base* base;
	RENDERER_BACKEND_STRUCTURES(device);
};

struct renderer_backend_swapchain
{
	RENDERER_BACKEND_STRUCTURES(swapchain);
};

struct renderer_backend_info* renderer_backend_get_info(
	struct minec_client* client,
	uint32_t backend_index
);

uint32_t renderer_backend_base_create(
	struct minec_client* client, 
	uint32_t backend_index, 
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos, 
	struct renderer_backend_base* base
);
void renderer_backend_base_destroy(
	struct minec_client* client, 
	struct renderer_backend_base* base
);

uint32_t renderer_backend_device_create(
	struct minec_client* client, 
	uint32_t device_index, 
	struct renderer_backend_base* base,
	struct renderer_backend_device* device
);
void renderer_backend_device_destroy(
	struct minec_client* client, 
	struct renderer_backend_device* device
);

uint32_t renderer_backend_swapchain_create(
	struct minec_client* client,
	struct renderer_backend_device* device,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering,
	struct renderer_backend_swapchain* swapchain
);
void renderer_backend_swapchain_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device,
	struct renderer_backend_swapchain* swapchain
);

uint32_t renderer_backend_frame_start(
	struct minec_client* client,
	struct renderer_backend_device* device
);

uint32_t renderer_backend_frame_submit(
	struct minec_client* client,
	struct renderer_backend_device* device
);

#endif