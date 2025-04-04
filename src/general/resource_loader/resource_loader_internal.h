#pragma once

#ifndef RESOURCE_LOADER_INTERNAL_H
#define RESOURCE_LOADER_INTERNAL_H

#include "resource_loader.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "general/platformlib/platform/platform.h"
#include "general/string_allocator.h"

#define _RESOURCE_LOADER_ITEMS_PER_EXTENSION 256

struct resource_loader_item
{
	bool in_use;

	uint8_t* token;
	uint8_t* groups[RESOURCE_LOADER_MAX_GROUPS_PER_ITEM];

	uint8_t* path;
	uint8_t* loader;

	bool loaded;
	void* resource_data;
	size_t resource_data_size;
};

struct resource_loader
{
	void* string_allocator;

	struct resource_loader_item* items;
	uint32_t item_count;
};

#endif // !RESOURCE_LOADER_INTERNAL_H
