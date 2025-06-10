#pragma once

#ifndef MINEC_CLIENT_RENDERER_RENDERER_H
#define MINEC_CLIENT_RENDERER_RENDERER_H

#define RENDRER_MAX_BACKEND_COUNT 8
#define RENDERER_MAX_BACKEND_DEVICE_COUNT 16

struct renderer_settings_state
{
	uint32_t backend_index;
	uint32_t backend_device_index;

	uint32_t fps;
	bool vsync;
};

struct renderer_info_state
{
	struct renderer_backend_info
	{
		uint8_t name[64];
	} backned_infos[RENDRER_MAX_BACKEND_COUNT];

	struct renderer_backend_device_info
	{
		uint8_t name[64];
		uint8_t version[64];

		bool usable;
		bool disable_vsync_support_support;

		uint8_t backend_device_specific_info[256];
	} backend_device_infos[RENDERER_MAX_BACKEND_DEVICE_COUNT];

	uint32_t renderer_message;
};

struct renderer_internal_state;

struct renderer
{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	void* library_handle;
	uint8_t* library_path;
	uint8_t* library_copy_path;

	struct renderer_settings_state* settings_state_mirror;
#endif

	struct renderer_internal_interface
	{
		uint32_t(*renderer_create)(
			struct minec_client* client,
			struct renderer_settings_state* request_settings_state,
			struct renderer_info_state** info_state,
			struct renderer_settings_state** settings_state
		);
		void (*renderer_destroy)(struct minec_client* client);

		void (*renderer_switch_backend)(struct minec_client* client, uint32_t backend_index);
		void (*renderer_switch_backend_device)(struct minec_client* client, uint32_t backend_device_index);
		void (*renderer_set_fps)(struct minec_client* client, uint32_t fps);
		void (*renderer_set_vsync)(struct minec_client* client, bool vsync);
	} internal;

	struct renderer_internal_state* renderer_internal_state;
};

struct minec_client;

uint32_t renderer_create(
	struct minec_client* client, 
	struct renderer_settings_state* request_settings_state, 
	struct renderer_info_state** info_state, 
	struct renderer_settings_state** settings_state
);
void renderer_destroy(struct minec_client* client);

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
uint32_t renderer_reload(
	struct minec_client* client,
	struct renderer_info_state** info_state,
	struct renderer_settings_state** settings_state
);
#endif 

void renderer_switch_backend(struct minec_client* client, uint32_t backend_index);
void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index);
void renderer_set_fps(struct minec_client* client, uint32_t fps);
void renderer_set_vsync(struct minec_client* client, bool vsync);

#endif