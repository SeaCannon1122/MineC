#pragma once

#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdint.h>

void* array_list_string_new(uint32_t extension_element_count);
void array_list_string_delete(void* array_list_string);

uint32_t array_list_string_get_length(void* array_list_string);
const uint8_t* array_list_string_get_element(void* array_list_string, uint32_t index);

void array_list_string_set_element(void* array_list_string, uint32_t index, uint8_t* value);
void array_list_string_insert_element(void* array_list_string, uint32_t index, uint8_t* value);
void array_list_string_move_elements(void* array_list_string, uint32_t dst_index, uint32_t src_index, uint32_t element_count);
void array_list_string_swap_elements(void* array_list_string, uint32_t index0, uint32_t index1, uint32_t element_count);
void array_list_string_remove_elements(void* array_list_string, uint32_t start_index_inc, uint32_t end_index_inc);

void array_list_string_read_file_data(void* array_list_string, const uint8_t* file_data, size_t file_data_size);
uint8_t* array_list_string_write_file_data(void* array_list_string, size_t* file_data_size);

#endif