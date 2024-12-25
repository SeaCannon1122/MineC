#include "key_value.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define _ENTRY_EXPANSION_SIZE 100
#define _STRINGS_EXPANSION_SIZE 1000

struct key_value_meta_data {
    uint32_t maps_block_length;
    uint32_t next_map_index;
    uint32_t strings_block_size;
    uint32_t next_string_index;
};

struct key_value_entry {
    uint32_t type;
    uint32_t key;
    union {
        int64_t integer;
        float floating;
        uint32_t string;
    } value;
};

void* _key_value_expand_entries(void* key_value_map, uint32_t count) {
    struct key_value_meta_data* meta_data = key_value_map;
    uint8_t* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + (meta_data->maps_block_length + count) * sizeof(struct key_value_entry) + meta_data->strings_block_size);
    meta_data = key_value_map;
    meta_data->maps_block_length += count;
    strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
    memmove(strings, strings - count * sizeof(struct key_value_entry), meta_data->strings_block_size);

    return key_value_map;
}

void* _key_value_add_string(void* key_value_map, uint8_t* string, uint32_t* stringdex) {

    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    uint32_t length = strlen(string);
    if (meta_data->strings_block_size - meta_data->next_string_index < length + 1) {
        key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + sizeof(struct key_value_entry) * meta_data->maps_block_length + meta_data->strings_block_size + (length + 1) + _STRINGS_EXPANSION_SIZE);
        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
        meta_data->strings_block_size += (length + 1) + _STRINGS_EXPANSION_SIZE;
    }

    *stringdex = meta_data->next_string_index;

    for (uint32_t i = 0; i < length + 1; i++) {
        strings[meta_data->next_string_index] = string[i];
        meta_data->next_string_index++;
    }

    return key_value_map;
}

enum key_value_return_type key_value_set_integer(void** key_value_map, uint8_t* name, int64_t value) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {
        maps[map_index].value.integer = value;

        if (maps[map_index].type == VALUE_TYPE_INTEGER) return KEY_VALUE_INFO_DEFAULT;
        else {
            maps[map_index].type = VALUE_TYPE_INTEGER;
            return KEY_VALUE_INFO_CHANGED_TYPE;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, _ENTRY_EXPANSION_SIZE);
    }
    uint32_t key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = VALUE_TYPE_INTEGER;
    maps[meta_data->next_map_index].value.integer = value;

    meta_data->next_map_index++;
    return KEY_VALUE_INFO_ADDED_PAIR;
}

enum key_value_return_type key_value_set_float(void** key_value_map, uint8_t* name, float value) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {
        maps[map_index].value.floating = value;

        if (maps[map_index].type == VALUE_TYPE_FLOAT) return KEY_VALUE_INFO_DEFAULT;
        else {
            maps[map_index].type = VALUE_TYPE_FLOAT;
            return KEY_VALUE_INFO_CHANGED_TYPE;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, _ENTRY_EXPANSION_SIZE);
    }
    uint32_t key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = VALUE_TYPE_FLOAT;
    maps[meta_data->next_map_index].value.floating = value;

    meta_data->next_map_index++;
    return KEY_VALUE_INFO_ADDED_PAIR;
}

enum key_value_return_type key_value_set_string(void** key_value_map, uint8_t* name, uint8_t* value) {
    uint32_t value_index;
    *key_value_map = _key_value_add_string(*key_value_map, value, &value_index);

    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {      
        maps[map_index].value.string = value_index;

        if (maps[map_index].type == VALUE_TYPE_STRING) return KEY_VALUE_INFO_DEFAULT;
        else {
            maps[map_index].type = VALUE_TYPE_STRING;
            return KEY_VALUE_INFO_CHANGED_TYPE;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, _ENTRY_EXPANSION_SIZE);
    }
    uint32_t key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = VALUE_TYPE_STRING;
    maps[meta_data->next_map_index].value.string = value_index;

    meta_data->next_map_index++;
    return KEY_VALUE_INFO_ADDED_PAIR;
}

enum key_value_return_type key_value_get_integer(void** key_value_map, uint8_t* name, int64_t default_value, int64_t* buffer) {

    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == VALUE_TYPE_INTEGER) {
        *buffer = maps[map_index].value.integer;
        return KEY_VALUE_INFO_DEFAULT;
    }

    *buffer = default_value;
    return key_value_set_integer(key_value_map, name, default_value);
}
uint32_t key_value_get_float(void** key_value_map, uint8_t* name, float default_value, float* buffer) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
    
    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == VALUE_TYPE_FLOAT) {
        *buffer = maps[map_index].value.floating;
        return KEY_VALUE_INFO_DEFAULT;
    }

    *buffer = default_value;
    return key_value_set_float(key_value_map, name, default_value);
}


enum key_value_return_type key_value_get_string(void** key_value_map, uint8_t* name, uint8_t* default_value, uint8_t* buffer, size_t buffer_size) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (uint32_t map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == VALUE_TYPE_STRING) {
        uint32_t string_length = strlen(&strings[maps[map_index].value.string]) + 1;
        if (buffer_size < string_length) return KEY_VALUE_ERROR_BUFFER_TOO_SMALL;
        memcpy(buffer, &strings[maps[map_index].value.string], string_length);
        return KEY_VALUE_INFO_DEFAULT;
    }

    uint32_t string_length = strlen(default_value) + 1;
    if (buffer_size < string_length) return KEY_VALUE_ERROR_BUFFER_TOO_SMALL;
    memcpy(buffer, default_value, string_length);

    return key_value_set_string(key_value_map, name, default_value);
}

void* key_value_new(size_t maps_block_length, size_t strings_block_size) {

    struct key_value_meta_data* map = malloc(sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry) + strings_block_size);

    map->maps_block_length = maps_block_length;
    map->next_map_index = 0;
    map->strings_block_size = strings_block_size;
    map->next_string_index = 0;

    return map;
}

uint32_t key_value_get_size(void* key_value_map) {
    return ((struct key_value_meta_data*)key_value_map)->next_map_index;
}

uint32_t key_value_get_pair(void* key_value_map, struct key_value_pair* pair, uint32_t index) {
    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    uint8_t* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    if (meta_data->maps_block_length <= index) return KAY_VALUE_ERROR_INDEX_OUT_OF_BOUNDS;

    pair->type = maps[index].type;
    pair->key = &strings[maps[index].key];

    if (pair->type == VALUE_TYPE_STRING) {
        pair->value.string = &strings[maps[index].value.string];
    }
    else pair->value.integer = maps[index].value.integer;

    return KEY_VALUE_INFO_DEFAULT;
}

void key_value_combind(void** key_value_map_main, void** key_value_map_additum) {

    uint32_t size = ((struct key_value_meta_data*)*key_value_map_additum)->next_map_index;

    for (uint32_t i = 0; i < size; i++) {

        struct key_value_pair pair;
        key_value_get_pair(*key_value_map_additum, &pair, i);

        switch (pair.type)
        {
        case VALUE_TYPE_INTEGER: key_value_set_integer(key_value_map_main, pair.key, pair.value.integer); break;
        case VALUE_TYPE_FLOAT: key_value_set_float(key_value_map_main, pair.key, pair.value.floating); break;
        case VALUE_TYPE_STRING: key_value_set_string(key_value_map_main, pair.key, pair.value.string); break;
        }

    }

}

enum key_value_return_type key_value_load_yaml(void** key_value_map, uint8_t* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) return KEY_VALUE_ERROR_COULDNT_OPEN_FILE;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file) + 1;
    rewind(file);

    uint8_t* carrige_return_buffer = (uint8_t*)malloc(fileSize);

    size_t bytesRead = fread(carrige_return_buffer, sizeof(char), fileSize, file);

    fclose(file);

    carrige_return_buffer[fileSize - 1] = 0;

    uint8_t* buffer = (uint8_t*)malloc(fileSize);

    uint32_t offset = 0;

    for (uint32_t i = 0; i < fileSize; i++) {
        if (carrige_return_buffer[i] == '\r') offset++;
        else buffer[i - offset] = carrige_return_buffer[i];
    }

    free(carrige_return_buffer);

    fileSize -= offset;

    void* temp_key_value_map = key_value_new(_ENTRY_EXPANSION_SIZE, _STRINGS_EXPANSION_SIZE);

    uint32_t line_breaks = 0;
    for (uint32_t i = 0; i < fileSize - line_breaks; i++) if (buffer[i] == '\n') line_breaks++;

    uint32_t text_i = 0;
    for (uint32_t lines_i = 0; lines_i < line_breaks + 1; lines_i++) {
#define skip_spaces while(buffer[text_i] == ' ') text_i++;    

        skip_spaces
        if (buffer[text_i] == '\n' || buffer[text_i] == '\0') { text_i++;  continue; }

        uint32_t key_start = text_i;
        for (; buffer[text_i] != ' ' && buffer[text_i] != ':' && buffer[text_i] != '\n' && buffer[text_i] != '\0'; text_i++);
        uint32_t key_end = text_i;
        if (key_end - key_start == 0) goto _error;
        
        skip_spaces
        if (buffer[text_i] != ':') goto _error;
        text_i++;
        buffer[key_end] = '\0';

        skip_spaces

        uint32_t type = VALUE_TYPE_INTEGER;
        uint32_t string_quotes = 0;

        uint32_t value_start = text_i;

        for (; buffer[text_i] != '\n' && buffer[text_i ] != '\0' && (buffer[text_i] != ' ' || string_quotes == 1); text_i++) {
            if (buffer[text_i] == '"') { type = VALUE_TYPE_STRING; string_quotes ^= 1; }
            if (type == VALUE_TYPE_STRING) continue;
            if (buffer[text_i] == '.' && type == VALUE_TYPE_INTEGER) type = VALUE_TYPE_FLOAT;
            else if ((buffer[text_i] < '0' || buffer[text_i] > '9') && (buffer[text_i] != '-' || text_i != value_start)) { type = VALUE_TYPE_STRING; }
        }

        if (type == VALUE_TYPE_STRING) {
            if (buffer[text_i - 1] == '"' && string_quotes == 0) {
                buffer[text_i - 1] = '\0';
                key_value_set_string(&temp_key_value_map, &buffer[key_start], &buffer[value_start + 1]);
            }
            else {
                buffer[text_i] = '\0';
                key_value_set_string(&temp_key_value_map, &buffer[key_start], &buffer[value_start]);
            }
        }
        else if (type == VALUE_TYPE_INTEGER) {
            uint64_t int_val = 0;
            uint32_t negative = 1;

            if (buffer[value_start] == '-') { value_start++; negative = -1; }
            for (uint32_t _i = value_start; _i < text_i; _i++) {
                int_val *= 10;
                int_val += buffer[_i] - '0';
            }
            int_val *= negative;
            key_value_set_integer(&temp_key_value_map, &buffer[key_start], int_val);
        }
        else {
            float float_val = 0;
            uint32_t decimal = -1;
            float negative = 1.f;

            if (buffer[value_start] == '-') { value_start++; negative = -1.f; }

            for (uint32_t _i = value_start; _i < text_i; _i++) {
                if (buffer[_i] == '.') decimal = _i;
                else if (decimal == -1) {
                    float_val *= 10;
                    float_val += buffer[_i] - '0';
                }
                else {
                    float_val += (float)(buffer[_i] - '0') / powf(10, _i - decimal);
                }
            }
            float_val *= negative;
            key_value_set_float(&temp_key_value_map, &buffer[key_start], float_val);
        }

        skip_spaces

        if (buffer[text_i] != '\n' && buffer[text_i] != '\0') goto _error;

        text_i++;

    }

    key_value_combind(key_value_map, &temp_key_value_map);

    free(temp_key_value_map);
    free(buffer);
    return KEY_VALUE_INFO_DEFAULT;

_error:
    free(temp_key_value_map);
    free(buffer);
    return KEY_VALUE_ERROR_FILE_INVALID_SYNTAX;
}

enum key_value_return_type key_value_write_yaml(void** key_value_map, uint8_t* file_path) {
    
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    uint32_t text_buffer_size = 1000;
    uint8_t* text_buffer = malloc(text_buffer_size);

    uint8_t* strings = (size_t)*key_value_map + meta_data->maps_block_length * sizeof(struct key_value_entry) + sizeof(struct key_value_meta_data);
    uint32_t text_i = 0;

    for (uint32_t map_i = 0; map_i < meta_data->next_map_index; map_i++) {
        
        {
            uint32_t _string_length = strlen(&strings[maps[map_i].key]);
            if (_string_length > text_buffer_size - text_i - 30) {
                text_buffer_size += _string_length + 200;
                text_buffer = realloc(text_buffer, text_buffer_size);
            }
            for (uint32_t _i = 0; _i < _string_length; _i++) { text_buffer[text_i] = strings[maps[map_i].key + _i]; text_i++; }
        }

        text_buffer[text_i] = ':';
        text_i++;
        text_buffer[text_i] = ' ';
        text_i++;

        if (maps[map_i].type == VALUE_TYPE_INTEGER) {
            uint32_t divisor_max = 1;
            for (; maps[map_i].value.integer / divisor_max >= 10; divisor_max *= 10);
            for (; divisor_max > 0; divisor_max /= 10) {
                text_buffer[text_i] = '0' + (maps[map_i].value.integer / divisor_max) % 10; text_i++;
            }
        }
        else if (maps[map_i].type == VALUE_TYPE_FLOAT) {
            uint32_t divisor_max = 1;
            uint32_t int_part = maps[map_i].value.floating;
            for (; int_part / divisor_max >= 10; divisor_max *= 10);
            for (; divisor_max > 0; divisor_max /= 10) {
                text_buffer[text_i] = '0' + (int_part / divisor_max) % 10; text_i++;
            }
            text_buffer[text_i] = '.';
            text_i++;
            float float_part = maps[map_i].value.floating - (float)int_part;

            uint32_t fractor = 10;
            for (; ((float)(int)(float_part * (float)fractor)) - (float_part * (float)fractor) != 0; fractor *= 10) {
                text_buffer[text_i] = '0' + (int)(float_part * (float)fractor) % 10; text_i++;
            }

        }
        else {
            text_buffer[text_i] = '"';
            text_i++;

            uint32_t string_length = strlen(&strings[maps[map_i].value.string]);
            if (string_length > text_buffer_size - text_i - 30) {
                text_buffer_size += string_length + 200;
                text_buffer = realloc(text_buffer, text_buffer_size);
            }
            for (uint32_t _i = 0; _i < string_length; _i++) {
                text_buffer[text_i] = strings[maps[map_i].value.string + _i];
                text_i++;
            }

            text_buffer[text_i] = '"';
            text_i++;

        }


        text_buffer[text_i] = '\n';
        text_i++;

    }

    FILE* file = fopen(file_path, "w");

    if (file == NULL) {

        free(text_buffer);
        return KEY_VALUE_ERROR_COULDNT_OPEN_FILE;
    }

    fwrite(text_buffer, sizeof(char), text_i, file);
    fclose(file);

    free(text_buffer);

    return KEY_VALUE_INFO_DEFAULT;
}