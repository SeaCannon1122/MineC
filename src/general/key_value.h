#pragma once

#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include <stdint.h>
#include <stdlib.h>

enum key_value_return_type {
    KAY_VALUE_ERROR_INDEX_OUT_OF_BOUNDS = 0b10001,
    KEY_VALUE_ERROR_COULDNT_OPEN_FILE = 0b10010,
    KEY_VALUE_ERROR_FILE_INVALID_SYNTAX = 0b10100,
    KEY_VALUE_ERROR_BUFFER_TOO_SMALL = 0b11000,
    KEY_VALUE_ERROR_MASK = 0b10000,
    KEY_VALUE_INFO_DEFAULT = 0b0,
    KEY_VALUE_INFO_CHANGED_TYPE = 0b1,
    KEY_VALUE_INFO_ADDED_PAIR = 0b10,
};

enum key_value_type {
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_STRING,
};

struct key_value_pair {
    uint32_t type;
    uint8_t* key;
    union {
        uint64_t integer;
        float floating;
        uint8_t* string;
    } value;
};

enum key_value_return_type key_value_set_integer(void** key_value_map, uint8_t* name, int64_t value);
enum key_value_return_type key_value_set_float(void** key_value_map, uint8_t* name, float value);
enum key_value_return_type key_value_set_string(void** key_value_map, uint8_t* name, uint8_t* value);

enum key_value_return_type key_value_get_integer(void** key_value_map, uint8_t* name, int64_t default_value, int64_t* buffer);
enum key_value_return_type key_value_get_float(void** key_value_map, uint8_t* name, float default_value, float* buffer);
enum key_value_return_type key_value_get_string(void** key_value_map, uint8_t* name, uint8_t* default_value, uint8_t* buffer, size_t buffer_size);

void* key_value_new(size_t maps_block_length, size_t strings_block_size);
uint32_t key_value_get_size(void* key_value_map);
uint32_t key_value_get_pair(void* key_value_map, struct key_value_pair* pair, uint32_t index);
void key_value_combind(void** key_value_map_main, void** key_value_map_additum);

enum key_value_return_type key_value_load_yaml(void** key_value_map, uint8_t* file_path);
enum key_value_return_type key_value_write_yaml(void** key_value_map, uint8_t* file_path);

#endif // !KEY_VALUE_H