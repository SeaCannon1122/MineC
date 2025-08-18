#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
#include "opengl/backend_opengl.h"
#endif

#define RENDRER_MAX_BACKEND_COUNT 8
#define RENDERER_MAX_BACKEND_DEVICE_COUNT 8

#define RENDERER_BACKEND_EXISTS RENDERER.backend.exists
#define RENDERER_BACKEND_FRAME_RESET RENDERER.backend.frame_reset

struct renderer_backend_info
{
	uint8_t name[64];
};

struct renderer_backend_device_info
{
	uint8_t name[64];
	uint8_t version[64];

	bool usable;
	bool disable_vsync_support;
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

struct renderer_backend_settings
{
	uint32_t backend_index;
	uint32_t backend_device_index;
	uint32_t fps;
	bool vsync;
	uint32_t max_mipmap_level_count;
};

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

struct renderer_backend
{
	bool exists;

	struct renderer_backend_device_infos device_infos;
	bool device_infos_changed;

	struct renderer_backend_settings settings;
	struct renderer_backend_settings settings_frame_backup;

	union renderer_backend_internal_state
	{
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
		struct renderer_backend_opengl opengl;
#endif
	} state;

	uint32_t pixelchar_slot;
	uint32_t pixelchar_slot_offset;
};

#endif

struct minec_client;

uint32_t renderer_backend_create(struct minec_client* client);
void renderer_backend_destroy(struct minec_client* client);

void renderer_backend_switch(struct minec_client* client, uint32_t index);
void renderer_backend_switch_device(struct minec_client* client, uint32_t device_index);
void renderer_backend_reload_resources(struct minec_client* client);
void renderer_backend_set_vsync(struct minec_client* client, bool vsync);
void renderer_backend_set_fps(struct minec_client* client, uint32_t fps);
void renderer_backend_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);

void renderer_backend_frame_begin(struct minec_client* client);
void renderer_backend_frame_menu(struct minec_client* client);
void renderer_backend_frame_end(struct minec_client* client);

#endif