#pragma once

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>

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
		uint32_t _int;
		float _float;
	} data;
	uint32_t type;
};


void* hashmap_new(uint32_t sub_array_count, uint32_t subarray_extension_mappings_count);
void hashmap_delete(void* hashmap);

void hashmap_set_value(void* hashmap, uint8_t* key, void* value, uint32_t value_type);
struct hashmap_multi_type* hashmap_get_value(void* hashmap, uint8_t* key);
void hashmap_delete_key(void* hashmap, uint8_t* key);

#endif // !HASHMAP_H
