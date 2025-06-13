#pragma once

#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdbool.h>
#include <pixelchar/pixelchar.h>

struct resources_texture
{
	uint32_t width;
	uint32_t height;
	void* data;
};

struct resources_pixelchar_font
{
	void* font_file_data;
	size_t font_file_data_size;
};

struct resources_key_value_map
{
	void* map;

	bool in_use;
};

struct resources_shader
{
	struct
	{
		void* data;
		size_t data_size;
	} shader;

	bool in_use;
};

struct resource_index
{
	struct resources_texture* textures;
	uint32_t texture_count;
	void* texture_token_id_hashmap;

	struct resources_pixelchar_font* pixelchar_fonts;
	uint32_t pixelchar_font_count;
	void* pixelchar_font_token_id_hashmap;

	void** language_hashmaps;
	uint32_t language_hashmap_count;
	void* language_hashmap_id_hashmap;
};

struct minec_client;

void resources_create(struct minec_client* client);

void resources_destroy(struct minec_client* client);

#endif // !RESOURCES_H
