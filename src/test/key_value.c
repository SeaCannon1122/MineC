#include "key_value.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


enum key_value_type {
    _VALUE_TYPE_INTEGER,
    _VALUE_TYPE_FLOAT,
    _VALUE_TYPE_STRING,
};

int key_value_get_integer(void* key_value_map, char* name, int default_value, int* buffer) {

    struct key_value_meta_data* meta_data = key_value_map;
    char* strings = (size_t)key_value_map + meta_data->strings_block_begin;
    int map_index = 0;
    for (; map_index < meta_data->maps_count && strcmp(name, &strings[((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->maps_count) {
        if (((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].type == _VALUE_TYPE_INTEGER) {
            
            *buffer = ((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].value.integer;
            return 0;
        }
        return 2;
    }

    return 1;
}
int key_value_get_float(void* key_value_map, char* name, float default_value, float* buffer) {
    struct key_value_meta_data* meta_data = key_value_map;
    char* strings = (size_t)key_value_map + meta_data->strings_block_begin;
    int map_index = 0;
    for (; map_index < meta_data->maps_count && strcmp(name, &strings[((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->maps_count) {
        if (((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].type == _VALUE_TYPE_FLOAT) {

            *buffer = ((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].value.floating;
            return 0;
        }
        return 2;
    }

    return 1;
}
int key_value_get_string(void* key_value_map, char* name, char* default_value, char* buffer, int buffer_size) {
    struct key_value_meta_data* meta_data = key_value_map;
    char* strings = (size_t)key_value_map + meta_data->strings_block_begin;
    int map_index = 0;
    for (; map_index < meta_data->maps_count && strcmp(name, &strings[((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->maps_count) {
        if (((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].type == _VALUE_TYPE_STRING) {

            int string_length = 1;

            char* string_ptr = &strings[((struct key_value_entry*)((size_t)key_value_map + sizeof(struct key_value_meta_data)))[map_index].value.string];

            for (; string_ptr[string_length - 1] != '\0'; string_length++);

            if (buffer_size < string_length) return 3;



            memcpy(buffer, string_ptr, string_length);

            return 0;
        }
        return 2;
    }

    return 1;
}

void* key_value_load_yaml(char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file) + 1;
    rewind(file);

    char* buffer = (char*)malloc(fileSize);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    fclose(file);

    int line_breaks = 0;
    for (int i = 0; i < fileSize - line_breaks; i++) if (buffer[i] == '\n') line_breaks++;

    buffer[fileSize - 1 - line_breaks] = 0;

    char* settings_map = malloc(sizeof(struct key_value_meta_data) + 10 * sizeof(struct key_value_entry) + 800);

    int map_block = sizeof(struct key_value_meta_data);
    int maps_count = 10;
    int map_block_next = 0;
    int strings_block = 10 * sizeof(struct key_value_entry) + sizeof(struct key_value_meta_data);
    int strings_block_size = 800;
    int strings_block_next = 0;

    int lines = 1;

    for (int i = 0; i < fileSize; i++) if (buffer[i] == '\n') lines++;

    int text_i = 0;
    for (int lines_i = 0; lines_i < lines; lines_i++) {
#define skip_spaces while(buffer[text_i] == ' ') text_i++;
#define token_length(value) for(int _i = 0; buffer[text_i + _i] != ' ' && buffer[text_i+ _i] != ':' && buffer[text_i + _i] != '\n' && buffer[text_i + _i] != '\0'; _i++) value++;

#define add_string(length, string_index) \
        if(strings_block_size - strings_block_next < length + 1) {\
            settings_map = realloc(settings_map, sizeof(struct key_value_meta_data) + sizeof(struct key_value_entry) * maps_count + strings_block_size + (length + 1) + 200);\
            strings_block_size = strings_block_size + (length + 1) + 200;\
        }\
        string_index = strings_block_next;\
        for (int _i = 0; _i < length; _i++) { settings_map[strings_block + strings_block_next] = buffer[text_i]; strings_block_next++; text_i++; }\
        settings_map[strings_block + strings_block_next] = '\0'; strings_block_next++;

        skip_spaces

            if (buffer[text_i] == '\n' || buffer[text_i] == '\0') continue;

        int key_length = 0;
        token_length(key_length)

            if (key_length == 0) goto _error;

        int key_index;
        add_string(key_length, key_index)

            skip_spaces

            if (buffer[text_i] != ':') goto _error;
        text_i++;
        skip_spaces

            int type = _VALUE_TYPE_INTEGER;

        int value_length = 0;

        int string_quotes = 0;

        for (; buffer[text_i + value_length] != '\n' && buffer[text_i + value_length] != '\0' && (buffer[text_i + value_length] != ' ' || string_quotes == 1) && (buffer[text_i + value_length] != '"' || string_quotes != 1); value_length++) {
            if (buffer[text_i + value_length] == '"') { type = _VALUE_TYPE_STRING; string_quotes = 1; }
            if (type == _VALUE_TYPE_STRING) continue;
            if (buffer[text_i + value_length] == '.' && type == _VALUE_TYPE_INTEGER) type = _VALUE_TYPE_FLOAT;
            else if (buffer[text_i + value_length] < '0' || buffer[text_i + value_length] > '9') { type = _VALUE_TYPE_STRING; }
        }

        int value;

        if (type == _VALUE_TYPE_STRING) {
            if (string_quotes) {
                text_i++;
                add_string(value_length - 1, value)
                    text_i++;
            }
            else { add_string(value_length, value) }
        }
        else if (type == _VALUE_TYPE_INTEGER) {
            int int_val = 0;
            int end = text_i + value_length;
            for (; text_i < end; text_i++) {
                int_val *= 10;
                int_val += buffer[text_i] - '0';
            }
            value = int_val;
        }
        else {
            float float_val = 0;
            int end = text_i + value_length;
            int decimal = -1;
            for (; text_i < end; text_i++) {
                if (buffer[text_i] == '.') decimal = text_i;
                else if (decimal == -1) {
                    float_val *= 10;
                    float_val += buffer[text_i] - '0';
                }
                else {
                    float_val += (buffer[text_i] - '0') / powf(10, text_i - decimal);
                }
            }

            *((float*)&value) = float_val;
        }

        skip_spaces

            if (buffer[text_i] != '\n' && buffer[text_i] != '\0') goto _error;

        text_i++;

        if (map_block_next == maps_count) {

            settings_map = realloc(settings_map, sizeof(struct key_value_meta_data) + (maps_count + 20) * sizeof(struct key_value_entry) + strings_block_size);
            memmove(settings_map + (maps_count + 20) * sizeof(struct key_value_entry), settings_map + maps_count * sizeof(struct key_value_entry), strings_block_size);
            maps_count += 20;
            strings_block += 20 * sizeof(struct key_value_entry);
        }

        ((struct key_value_entry*)(settings_map + sizeof(struct key_value_meta_data)))[map_block_next] = (struct key_value_entry){
            type,
            key_index,
            value,
        };

        map_block_next++;

    }

    struct key_value_meta_data* meta_data = settings_map;
    meta_data->maps_count = map_block_next;
    meta_data->map_block_begin = sizeof(struct key_value_meta_data);
    meta_data->strings_block_begin = strings_block;
    meta_data->strings_block_size = strings_block_size;

    free(buffer);
    return settings_map;

_error:
    free(buffer);
    free(settings_map);

    printf("error parsing yaml %s\n", file_path);

    return NULL;
}

void key_value_write_yaml(char* file_path, void* handle) {

}