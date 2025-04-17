#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "string_allocator.h"

uint32_t hash_djb2(const uint8_t* str, uint32_t max) {
	uint32_t hash = 5381;
	uint32_t c;

	while ((c = *(str++))) hash = ((hash << 5) + hash) + c;

	return hash % max;
}

struct _hashmap_entry
{
	uint8_t* key;
	union
	{
		uint8_t* string;
		size_t value;
	} value;
	bool in_use;
	bool is_string;
};

struct _hashmap_subarray
{
	uint32_t entry_count;
	struct _hashmap_entry* entries;
};

struct _hashmap
{
	void* string_allocator;

	struct _hashmap_subarray* sub_arrays;
	uint32_t sub_array_count;

	uint32_t subarray_extension_mappings_count;
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
	struct _hashmap* map = malloc(sizeof(struct _hashmap));

	map->sub_array_count = sub_array_count;
	map->subarray_extension_mappings_count = subarray_extension_mappings_count;
	map->sub_arrays = malloc(sizeof(struct _hashmap_subarray) * sub_array_count);
	memset(map->sub_arrays, 0, sizeof(struct _hashmap_subarray) * sub_array_count);
	map->string_allocator = string_allocator_new(16384);

	return map;
}

void hashmap_set_string(void* hashmap, uint8_t* key, uint8_t* string)
{
	struct _hashmap* map = hashmap;

	uint32_t index = hash_djb2(key, map->sub_array_count);
	uint32_t sub_index;
	
	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].is_string)
			string_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.string);
	}
	else
	{
		_hashmap_key_get_empty_space(map, key, index, &sub_index);
	}

	map->sub_arrays[index].entries[sub_index].is_string = true;
	map->sub_arrays[index].entries[sub_index].value.string = string_allocate_string(map->string_allocator, string);
}

void hashmap_set_data(void* hashmap, uint8_t* key, void* data, size_t data_size)
{
	struct _hashmap* map = hashmap;

	uint32_t index = hash_djb2(key, map->sub_array_count);
	uint32_t sub_index;

	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].is_string)
			string_free(map->string_allocator, map->sub_arrays[index].entries[sub_index].value.string);
	}
	else
	{
		_hashmap_key_get_empty_space(map, key, index, &sub_index);
	}

	map->sub_arrays[index].entries[sub_index].is_string = false;

	if (data_size == sizeof(uint8_t))
		*((uint8_t*)&map->sub_arrays[index].entries[sub_index].value.value) = *(uint8_t*)data;
	else if (data_size == sizeof(uint16_t))
		*((uint16_t*)&map->sub_arrays[index].entries[sub_index].value.value) = *(uint16_t*)data;
	else
		*((uint32_t*)&map->sub_arrays[index].entries[sub_index].value.value) = *(uint32_t*)data;
}

uint8_t* hashmap_get_string(void* hashmap, uint8_t* key)
{
	struct _hashmap* map = hashmap;

	uint32_t index = hash_djb2(key, map->sub_array_count);
	uint32_t sub_index;

	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].is_string == false) return NULL;

		return map->sub_arrays[index].entries[sub_index].value.string;
	}

	return NULL;
}

uint32_t hashmap_get_data(void* hashmap, uint8_t* key, void* data, size_t data_size)
{
	struct _hashmap* map = hashmap;

	uint32_t index = hash_djb2(key, map->sub_array_count);
	uint32_t sub_index;

	if (_hashmap_key_exists(map, key, index, &sub_index))
	{
		if (map->sub_arrays[index].entries[sub_index].is_string) return 0;

		if (data_size == sizeof(uint8_t))
			*(uint8_t*)data = *((uint8_t*)&map->sub_arrays[index].entries[sub_index].value.value);
		else if (data_size == sizeof(uint16_t))
			*(uint16_t*)data = *((uint16_t*)&map->sub_arrays[index].entries[sub_index].value.value);
		else
			*(uint32_t*)data = *((uint32_t*)&map->sub_arrays[index].entries[sub_index].value.value);

		return 1; 
	}

	return 0;
}

void hashmap_delete(void* hashmap)
{
	struct _hashmap* map = hashmap;

	string_allocator_delete(map->string_allocator);
	free(map->sub_arrays);
	free(map);
}