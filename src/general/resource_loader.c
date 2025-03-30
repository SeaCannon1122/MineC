#include "resource_loader.h"

#include <stdbool.h>
#include "string_allocator.h"

struct resource_loader_item
{
	bool exists;

	char* token;
	char* groups[RESOURCE_LOADER_MAX_GROUPS_PER_ITEM];

	bool loaded;
	char* path;
	char* loader;


	void* resource_data;
	size_t resource_data_size;
};

struct resource_loader
{
	void* string_allocator;

	struct resource_loader_resource* resources;
	uint32_t resources_count;
};

void* resource_loader_create()
{

}

uint32_t resource_loader_add_directory(void* rl, uint8_t* path)
{

}

uint32_t resource_loader_add_item(void* rl, uint8_t* token, uint8_t* path)
{

}

uint32_t resource_loader_load_item(void* rl, uint8_t* token)
{

}

uint32_t resource_loader_load_all(void* rl)
{

}

uint32_t resource_loader_unload_item(void* rl, uint8_t* token)
{

}

uint32_t resource_loader_unload_all(void* rl)
{

}

void resource_loader_destroy(void* rl)
{

}