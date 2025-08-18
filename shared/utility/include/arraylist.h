#pragma once

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdint.h>
#include <stddef.h>

void* arraylist_string_new(uint32_t extension_element_count);
void arraylist_string_delete(void* arraylist_string);
void* arraylist_string_new_copy(void* arraylist_string);

uint32_t arraylist_string_get_length(void* arraylist_string);
const uint8_t* arraylist_string_get_element(void* arraylist_string, uint32_t index);

void arraylist_string_set_element(void* arraylist_string, uint32_t index, uint8_t* value);
void arraylist_string_insert_element(void* arraylist_string, uint32_t index, uint8_t* value);
void arraylist_string_move_elements(void* arraylist_string, uint32_t dst_index, uint32_t src_index, uint32_t element_count);
void arraylist_string_swap_elements(void* arraylist_string, uint32_t index0, uint32_t index1, uint32_t element_count);
void arraylist_string_remove_elements(void* arraylist_string, uint32_t start_index_inc, uint32_t end_index_inc);

void arraylist_string_read_file_data(void* arraylist_string, const uint8_t* file_data, size_t file_data_size);
uint8_t* arraylist_string_write_file_data(void* arraylist_string, size_t* file_data_size);

#endif