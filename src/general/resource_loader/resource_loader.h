#pragma once

#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#define RESOURCE_LOADER_MAX_GROUPS_PER_ITEM 8
#define RESOURCE_LOADER_BASE_PATH_COUNT 16
#define RESOURCE_LOADER_NO_BASE_PATH

#include <stdint.h>
#include <stdbool.h>

struct resource_loader
{
	void* path_allocator;
	void* temp_allocator;

	uint8_t* base_paths[RESOURCE_LOADER_BASE_PATH_COUNT];

	struct resource_loader_item* items;
	uint32_t item_count;
};

#endif // !RESOURCE_LOADER_H
