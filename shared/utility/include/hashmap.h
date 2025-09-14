#pragma once

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum hashmap_value
{
	HASHMAP_VALUE_STRING_class		= 0x10,

	HASHMAP_VALUE_STRING8			= 0x11,
	HASHMAP_VALUE_STRING16			= 0x12,
	HASHMAP_VALUE_STRING32			= 0x13,


	HASHMAP_VALUE_INTEGER_class		= 0x20,

	HASHMAP_VALUE_INT8				= 0x21,
	HASHMAP_VALUE_UINT8				= 0x22,
	HASHMAP_VALUE_INT16				= 0x23,
	HASHMAP_VALUE_UINT16			= 0x24,
	HASHMAP_VALUE_INT32				= 0x25,
	HASHMAP_VALUE_UINT32			= 0x26,
	HASHMAP_VALUE_INT64				= 0x27,
	HASHMAP_VALUE_UINT64			= 0x28,


	HASHMAP_VALUE_FLOATING_class	= 0x40,

	HASHMAP_VALUE_FLOAT				= 0x41,
	HASHMAP_VALUE_DOUBLE			= 0x42,


	HASHMAP_VALUE_BOOLEAN_class		= 0x80,

	HASHMAP_VALUE_BOOL				= 0x81,
};

struct hashmap_multi_type
{
	union
	{
		uint8_t* data_string8;
		uint16_t* data_string16;
		uint32_t* data_string32;

		int8_t data_int8;
		uint8_t data_uint8;
		int16_t data_int16;
		uint16_t data_uint16;
		int32_t data_int32;
		uint32_t data_uint32;
		int64_t data_int64;
		uint64_t data_uint64;

		float data_float;
		double data_double;

		bool data_bool;
	};

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
uint8_t* hashmap_write_yaml(void* hashmap, size_t* yaml_data_size);

#endif
