#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#define RENDRER_MAX_BACKEND_COUNT 8
#define RENDERER_MAX_BACKEND_DEVICE_COUNT 8

#include <stdint.h>

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

struct renderer_backend_info_state
{
	struct renderer_backend_infos backend_infos;
	struct renderer_backend_device_infos backend_device_infos;
};

struct renderer_settings
{
	struct renderer_backend_settings
	{
		uint32_t backend_index;
		uint32_t backend_device_index;
		uint32_t fps;
		bool vsync;
		uint32_t max_mipmap_level_count;
	} backend;

	struct renderer_frontend_settings
	{
		uint32_t fov;
		uint32_t gui_scale;
		uint32_t render_distance;
	} frontend;

	struct renderer_other_settings
	{
		bool order_create_new_destroy_old;
	} other;
};

struct renderer_info_state
{
	struct renderer_backend_info_state backend;
	struct renderer_settings settings;
};


struct renderer_internal_state;

struct renderer
{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	void* library_handle;

	struct renderer_settings settings_mirror;
#endif

	struct renderer_api
	{
		uint32_t(*create)(struct minec_client* client,struct renderer_settings* settings);
		void(*destroy)(struct minec_client* client);
		bool(*did_crash)(struct minec_client* client);

		bool(*get_info_state)(struct minec_client* client, struct renderer_info_state* info_state);

		void(*set_order_create_new_destroy_old)(struct minec_client* client, bool order);
		void(*reload_resources)(struct minec_client* client);
		void(*switch_backend)(struct minec_client* client, uint32_t backend_index);
		void(*switch_backend_device)(struct minec_client* client, uint32_t backend_device_index);
		void(*set_vsync)(struct minec_client* client, bool vsync);
		void(*set_fps)(struct minec_client* client, uint32_t fps);
		void(*set_gui_scale)(struct minec_client* client, uint32_t scale);
		void(*set_fov)(struct minec_client* client, uint32_t fov);
		void(*set_render_distance)(struct minec_client* client, uint32_t render_distance);
		void(*set_max_mipmap_level_count)(struct minec_client* client, uint32_t max_mipmap_level_count);
	} api;

	struct renderer_internal_state* state;
};

struct minec_client;

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings);
void renderer_destroy(struct minec_client* client);
bool renderer_did_crash(struct minec_client* client);

bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state);

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
uint32_t renderer_reload(struct minec_client* client);
#endif 

void renderer_set_order_create_new_destroy_old(struct minec_client* client, bool order);
void renderer_switch_backend(struct minec_client* client, uint32_t backend_index);
void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index);
void renderer_reload_resources(struct minec_client* client);
void renderer_set_vsync(struct minec_client* client, bool vsync);
void renderer_set_fps(struct minec_client* client, uint32_t fps);
void renderer_set_gui_scale(struct minec_client* client, uint32_t scale);
void renderer_set_fov(struct minec_client* client, uint32_t fov);
void renderer_set_render_distance(struct minec_client* client, uint32_t render_distance);
void renderer_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count);

#endif