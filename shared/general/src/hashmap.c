#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "string_allocator.h"

uint32_t hash_djb2(const uint8_t* str, uint32_t max) {
	uint32_t c, hash = 5381;

	while ((c = *(str++))) hash = ((hash << 5) + hash) + c;

	return hash % max;
}

struct _hashmap_entry
{
	uint8_t* key;
	struct hashmap_multi_type value;
	bool in_use;
};

struct _hashmap_subarray
{
	uint32_t entry_count;
	struct _hashmap_entry* entries;
};

struct _hashmap
{
	void* string_allocator;

	uint32_t sub_array_count;
	uint32_t subarray_extension_mappings_count;

	struct _hashmap_subarray sub_arrays[];
};

uint32_t _hashmap_key_exists(struct _hashmap* map, uint8_t* key, uint32_t index, uint32_t* sub_index)
{
	for (uint32_t i = 0; i < map->sub_arrays[index].entry_count; i++)
	{
		if (map->sub_arrays[index].entries[i].in_use)
		{
			if (strcmp(key, map->sub_arrays[index].entries[i].key) == 0)
			{
				*sub_index = i;
				return 1;
			}
		}
	}
	return 0;
}

void _hashmap_key_get_empty_space(struct _hashmap* map, uint8_t* key, uint32_t index, uint32_t* sub_index)
{
	for (uint32_t i = 0; i < map->sub_arrays[index].entry_count; i++)
	{
		if (map->sub_arrays[index].entries[i].in_use == false)
		{
			map->sub_arrays[index].entries[i].in_use = true;
			map->sub_arrays[index].entries[i].key = string_allocate_string(map->string_allocator, key);
			*sub_index = i;
			return;
		}
	}

	uint32_t old_entry_count = map->sub_arrays[index].entry_count;
	map->sub_arrays[index].entry_count += map->subarray_extension_mappings_count;

	if (old_entry_count == 0)
		map->sub_arrays[index].entries = string_allocate(map->string_allocator, sizeof(struct _hashmap_entry) * map->sub_arrays[index].entry_count);
	else
	{
		struct _hashmap_entry* old_entries = map->sub_arrays[index].entries;
		map->sub_arrays[index].entries = string_allocate(map->string_allocator, sizeof(struct _hashmap_entry) * map->sub_arrays[index].entry_count);
		memcpy(map->sub_arrays[index].entries, old_entries, sizeof(struct _hashmap_entry) * old_entry_count);
		memset((size_t)map->sub_arrays[index].entries + sizeof(struct _hashmap_entry) * old_entry_count, 0, sizeof(struct _hashmap_entry) * map->subarray_extension_mappings_count);
		string_free(map->string_allocator, old_entries);
	}

	map->sub_arrays[index].entries[old_entry_count].in_use = true;
	map->sub_arrays[index].entries[old_entry_count].key = string_allocate_string(map->string_allocator, key);
	*sub_index = old_entry_count;

	return;
}

void* hashmap_new(uint32_t sub_array_count, uint32_t subarray_extension_mappings_count)
{
	struct _hashmap* map = malloc(sizeof(struct _hashmap) + sizeof(struct _hashmap_subarray) * sub_array_count);

	map->sub_array_count = sub_array_count;
	map->subarray_extension_mappings_count = subarray_extension_mappings_count;
	memset(map->sub_arrays, 0, sizeof(struct _hashmap_subarray) * sub_array_count);
	map->string_allocator = string_allocator_new(16384);

	return map;
}

void hashmap_delete(void* hashmap)
{
	struct _hashmap* map = hashmap;

	string_allocator_delete(map->string_allocator);
	free(map);
}

void hashmap_set_value(void* hashmap, uint8_t* key, void* value, uint32_t value_type)
{
	struct _hashmap* map = hashmap;

	struct hashmap_multi_type resolved_value;
	resolved_value.type = value_type;

	switch (value_type)
	{
	case HASHMAP_VALUE_STRING: {
		resolved_value.data._string = string_allocate_string(map->string_allocator, value);
	} break;
	
	case HASHMAP_VALUE_FLOAT : {
		resolved_value.data._float = *((float*)value);
	} break;

	case HASHMAP_VALUE_INT: {
		resolved_value.data._int = *((uint32_t*)value);
	} break;

	default: return;

	}

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);
	
	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].value.type == HASHMAP_VALUE_STRING)
			string_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.data._string);
	}
	else
		_hashmap_key_get_empty_space(map, key, index, &sub_index);

	map->sub_arrays[index].entries[sub_index].value = resolved_value;
}

struct hashmap_multi_type* hashmap_get_value(void* hashmap, uint8_t* key)
{
	struct _hashmap* map = hashmap;

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);

	if (_hashmap_key_exists(map, key, index, &sub_index))
		return &map->sub_arrays[index].entries[sub_index].value;

	return NULL;
}

void hashmap_delete_key(void* hashmap, uint8_t* key)
{
	struct _hashmap* map = hashmap;

	uint32_t sub_index, index = hash_djb2(key, map->sub_array_count);

	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].value.type == HASHMAP_VALUE_STRING)
			string_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.data._string);

		string_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].key);
		map->sub_arrays[index].entries[sub_index].in_use = false;
	}
}