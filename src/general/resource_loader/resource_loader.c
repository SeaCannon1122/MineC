#include "resource_loader_internal.h"

uint32_t _resource_loader_add_item(struct resource_loader* rl, uint8_t* token, uint8_t* path, uint8_t* loader, uint8_t* groups[RESOURCE_LOADER_MAX_GROUPS_PER_ITEM])
{
	for (uint32_t i = 0; i < rl->item_count; i++) if (rl->items[i].in_use = false)
	{
		rl->items[i].in_use = true;
		rl->items[i].token = string_allocate_string(rl->path_allocator, token);


		for (uint32_t j = 0; j < RESOURCE_LOADER_MAX_GROUPS_PER_ITEM; j++) {

			if (groups[j] != NULL)
				rl->items[i].groups[j] = string_allocate_string(rl->path_allocator, groups[j]);
			else
				rl->items[i].groups[j] = NULL;
		}

		rl->items[i].path = string_allocate_string(rl->path_allocator, path);
		rl->items[i].loader = string_allocate_string(rl->path_allocator, loader);
	
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

void resource_loader_create(struct resource_loader* rl)
{

	rl->path_allocator = string_allocator_new(4096);
	rl->temp_allocator = string_allocator_new(4096);

	rl->item_count = 0;

	memset(rl->base_paths, 0, sizeof(rl->base_paths))
}

uint32_t resource_loader_set_base_path(struct resource_loader* rl, uint8_t* path, uint32_t index)
{
	if (index >= RESOURCE_LOADER_BASE_PATH_COUNT) return 1;

	if (rl->base_paths[index] != NULL) string_free(rl->path_allocator, rl->base_paths[index]);

	rl->base_paths[index] = string_allocate_string(rl->path_allocator, path);
	
	return 0;
}

uint32_t resource_loader_add_layout_file(struct resource_loader* rl, uint8_t* path, bool load)
{

}

uint32_t resource_loader_add_item(struct resource_loader* rl, uint8_t* token, uint8_t* path, bool load )
{

}

uint32_t resource_loader_load_items(struct resource_loader* rl, uint32_t item_count, ...)
{

}

uint32_t resource_loader_load_groups(struct resource_loader* rl, uint32_t group_count, ...)
{

}

uint32_t resource_loader_load_all(struct resource_loader* rl)
{

}

uint32_t resource_loader_unload_items(struct resource_loader* rl, uint32_t item_count, ...)
{

}

uint32_t resource_loader_unload_groups(struct resource_loader* rl, uint32_t group_count, ...)
{

}

uint32_t resource_loader_unload_all(struct resource_loader* rl)
{

}

void resource_loader_destroy(struct resource_loader* rl)
{
	for (uint32_t i = 0; i < RESOURCE_LOADER_BASE_PATH_COUNT; i++) if (rl->base_paths[i] != NULL)
		string_free(rl->path_allocator, rl->base_paths[i]);

	string_allocator_delete(rl->path_allocator);
	string_allocator_delete(rl->temp_allocator);
}