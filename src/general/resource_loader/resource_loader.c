#include "resource_loader_internal.h"

uint32_t _resource_loader_add_item(struct resource_loader* rl, uint8_t* token, uint8_t* path, uint8_t* loader, uint8_t* groups[RESOURCE_LOADER_MAX_GROUPS_PER_ITEM])
{
	for (uint32_t i = 0; i < rl->item_count; i++) if (rl->items[i].in_use = false)
	{
		rl->items[i].in_use = true;
		rl->items[i].token = string_allocate_string(rl->string_allocator, token);


		for (uint32_t j = 0; j < RESOURCE_LOADER_MAX_GROUPS_PER_ITEM; j++) {

			if (groups[j] != NULL)
				rl->items[i].groups[j] = string_allocate_string(rl->string_allocator, groups[j]);
			else
				rl->items[i].groups[j] = NULL;
		}

		rl->items[i].path = string_allocate_string(rl->string_allocator, path);
		rl->items[i].loader = string_allocate_string(rl->string_allocator, loader);
	
		rl->items[i].loaded = false;

		return i;
	}

	rl->item_count += _RESOURCE_LOADER_ITEMS_PER_EXTENSION;
	if (rl->item_count == _RESOURCE_LOADER_ITEMS_PER_EXTENSION)
		rl->items = malloc(sizeof(struct resource_loader_item) * rl->item_count);
	else 
		rl->items = realloc(rl->items, sizeof(struct resource_loader_item) * rl->item_count);

	_resource_loader_add_item(rl, token, path, loader, groups);
}

void* resource_loader_create()
{
	struct resource_loader* resource_loader = (struct resource_loader*)malloc(sizeof(struct resource_loader));

	resource_loader->string_allocator = string_allocator_new(4096);
	resource_loader->item_count = 0;

	return resource_loader;
}

uint32_t resource_loader_add_layout_file(void* rl, uint8_t* path, bool load)
{

}

uint32_t resource_loader_add_item(void* rl, uint8_t* token, uint8_t* path, bool load )
{

}

uint32_t resource_loader_load_items(void* rl, uint32_t item_count, ...)
{

}

uint32_t resource_loader_load_groups(void* rl, uint32_t group_count, ...)
{

}

uint32_t resource_loader_load_all(void* rl)
{

}

uint32_t resource_loader_unload_items(void* rl, uint32_t item_count, ...)
{

}

uint32_t resource_loader_unload_groups(void* rl, uint32_t group_count, ...)
{

}

uint32_t resource_loader_unload_all(void* rl)
{

}

void resource_loader_destroy(void* rl)
{

}