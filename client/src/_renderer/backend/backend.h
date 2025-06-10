#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#define RENDERER_BACKEND_MAX_DEVICE_COUNT 16

struct renderer_backend_device_info
{
	uint8_t name[128];
	uint8_t version[128];

	bool usable;
	bool immediate_present_support;

	uint8_t backend_specific_info[512];
};

struct renderer_backend_settings_state
{
	uint32_t backend_index;
	uint32_t device_index;
};

struct renderer_backend_interface
{
	uint32_t (*base_create)(struct minec_client* client, void** base, uint32_t* device_count, struct renderer_backend_device_info** device_infos);
	void (*base_destroy)(struct minec_client* client, void** base);

	uint32_t (*device_create)(struct minec_client* client, void** base, void** device, uint32_t device_index, uint32_t fps, bool immediate_present);
	void (*device_destroy)(struct minec_client* client, void** base, void** device);

	uint32_t (*set_fps)(struct minec_client* client, uint32_t fps);

	uint32_t (*pipelines_resources_create)(struct minec_client* client, void** base, void** device, void** pipelines_resources, uint32_t pcr_backend_index);
	void (*pipelines_resources_destroy)(struct minec_client* client, void** base, void** device, void** pipelines_resources);

	uint32_t (*start_rendering)(struct minec_client* client);
	void (*stop_rendering)(struct minec_client* client);
	uint32_t (*render)(struct minec_client* client, bool resize);

	uint32_t (*create)(
		struct minec_client* client, 
		uint32_t backend_index,
		uint32_t device_index,
		uint32_t fps,
		bool vsync 

	);
};

#endif