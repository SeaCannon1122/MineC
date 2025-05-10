#pragma once

#ifndef MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H
#define MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H

#include <stdint.h>

struct settings {

	struct
	{
		uint32_t gui_scale;
		uint32_t fov;

		struct {
			uint32_t backend_index;
			uint8_t** backend_names;
			uint32_t backend_count;
			uint32_t device_index;
			uint8_t** device_infos;
			uint32_t device_count;

			uint32_t fps;
		} graphics;

	} video;

	void* resource_pack_paths_hashmap;
};

struct minec_client;

void settings_create(struct minec_client* client);
void settings_destroy(struct minec_client* client);

void settings_load(struct minec_client* client);
void settings_save(struct minec_client* client);

#endif // !SETTINGS_H
