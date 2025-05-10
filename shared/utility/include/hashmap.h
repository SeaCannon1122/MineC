#pragma once

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stddef.h>

enum hasmap_value
{
	HASHMAP_VALUE_STRING,
	HASHMAP_VALUE_FLOAT,
	HASHMAP_VALUE_INT,
};

struct hashmap_multi_type
{
	union
	{
		uint8_t* _string;
		int32_t _int;
		float _float;
	} data;
	uint32_t type;
};


void* hashmap_new(uint32_t sub_array_count, uint32_t subarray_extension_mappings_count);
void hashmap_delete(void* hashmap);

void hashmap_set_value(void* hashmap, uint8_t* key, void* value, uint32_t value_type);
struct hashmap_multi_type* hashmap_get_value(void* hashmap, uint8_t* key);
void hashmap_delete_key(void* hashmap, uint8_t* key);

uint32_t hashmap_get_key_count(void* hashmap);

struct hashmap_iterator
{
	void* map;
	uint32_t index;
	uint32_t sub_index;
};

void hashmap_iterator_start(struct hashmap_iterator* iterator, void* hashmap);
struct hashmap_multi_type* hashmap_iterator_next_key_value_pair(struct hashmap_iterator* iterator, uint8_t** key);

void hashmap_read_yaml(void* hashmap, uint8_t* yaml_data, size_t yaml_data_size);
void* hashmap_write_yaml(void* hashmap, size_t* yaml_data_size);

#endif // !HASHMAP_H
