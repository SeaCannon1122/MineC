#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "key_value.h"

struct resource_manager_image {
	uint32_t width;
	uint32_t height;
	uint32_t* data;
};

struct resource_manager_binary {
	uint8_t* data;
	uint32_t size;
};

struct resource_manager {

	uint32_t image_count;
	struct resource_manager_image* images;
	void* images_name_map;

	uint32_t binaries_count;
	struct resource_manager_binary* binaries;
	void* binaries_names_map;

	uint32_t key_value_count;
	void* key_value_maps;
	void* key_value_maps_names_map;
	void* key_value_maps_paths_map;

	uint32_t audio_count;
};

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path, int (*log_function)(const char* const, ...));

uint32_t resource_manager_destroy(struct resource_manager* rm);

uint32_t resource_manager_get_image(struct resource_manager* rm, uint8_t* name, struct resource_manager_image* image);
uint32_t resource_manager_get_binary(struct resource_manager* rm, uint8_t* name, struct resource_manager_binary* binary);
uint32_t resource_manager_get_key_value_map(struct resource_manager* rm, uint8_t* name, void** key_value_map);
uint32_t resource_manager_save_key_value_map(struct resource_manager* rm, uint8_t* name, void* key_value_map);

#endif // !RESOURCE_MANAGER_H

