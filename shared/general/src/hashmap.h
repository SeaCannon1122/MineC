#pragma once

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stddef.h>

void* hashmap_new(uint32_t sub_array_count, uint32_t subarray_extension_mappings_count);
void hashmap_delete(void* hashmap);

void hashmap_set_string(void* hashmap, uint8_t* key, uint8_t* string);
void hashmap_set_data(void* hashmap, uint8_t* key, void* data, size_t data_size);

uint8_t* hashmap_get_string(void* hashmap, uint8_t* key);
uint32_t hashmap_get_data(void* hashmap, uint8_t* key, void* data, size_t data_size);

#endif // !HASHMAP_H
