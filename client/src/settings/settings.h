#pragma once

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

struct settings_state {

	struct
	{
		struct
		{
			uint32_t gui_scale;
			uint32_t fov;

		} video_settings;

	} game_settings;

	void* resource_pack_paths_hashmap;
};

struct minec_client;

void settings_create(struct minec_client* client);
void settings_destroy(struct minec_client* client);

void settings_load(struct minec_client* client);
void settings_save(struct minec_client* client);

#endif // !SETTINGS_H
