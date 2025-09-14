#pragma once

#ifndef MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H
#define MINEC_CLIENT_SETTINGS_SETTINGS_SETTINGS_H

#define SETTINGS client->settings

#include <stdint.h>

#include <renderer/renderer.h>

struct settings {

	struct
	{
		struct renderer_info_state renderer_info;
		struct renderer_settings renderer;
		uint32_t gui_scale;
	} video;

	struct
	{
		struct
		{
			uint32_t count;
			struct string_index_language_info* infos;
			uint32_t index;
		} language;

	} general;

	uint8_t* general_settings_file_path;
	uint8_t* video_settings_file_path;
	uint8_t* active_resourcepack_file_path;
	uint8_t* inactive_resourcepack_file_path;

	void* active_resourcepack_paths_arraylist;
	void* inactive_resourcepack_paths_arraylist;
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE
struct minec_client;

uint32_t settings_create(struct minec_client* client);
void settings_destroy(struct minec_client* client);

void settings_load(struct minec_client* client);
void settings_save(struct minec_client* client);
#endif

#endif // !SETTINGS_H
