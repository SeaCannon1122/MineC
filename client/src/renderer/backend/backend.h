#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_BACKEND_H
#define MINEC_CLIENT_RENDERER_BACKEND_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

struct renderer_backend_interface
{
	uint32_t (*base_create)(struct minec_client* client, void** base, uint32_t* device_count, uint8_t*** device_infos);
	void (*base_destroy)(struct minec_client* client, void** base);

	uint32_t (*device_create)(struct minec_client* client, void** base, void** device, uint32_t device_index, uint32_t fps);
	void (*device_destroy)(struct minec_client* client, void** base, void** device);

	uint32_t (*set_fps)(struct minec_client* client, uint32_t fps);

	uint32_t (*pipelines_resources_create)(struct minec_client* client);
	void (*pipelines_resources_destroy)(struct minec_client* client);

	uint32_t (*render)(struct minec_client* client);
};

#endif