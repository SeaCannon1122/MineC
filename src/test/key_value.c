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

void* _key_value_expand_entries(void* key_value_map, int count) {
    struct key_value_meta_data* meta_data = key_value_map;
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + (meta_data->maps_block_length + count) * sizeof(struct key_value_entry) + meta_data->strings_block_size);
    meta_data = key_value_map;
    meta_data->maps_block_length += count;
    memmove(strings, strings - count * sizeof(struct key_value_entry), meta_data->strings_block_size);

    return key_value_map;
}

void* _key_value_add_string(void* key_value_map, char* string, int* stringdex) {

    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    int length = strlen(string);
    if (meta_data->strings_block_size - meta_data->next_string_index < length + 1) {
        key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + sizeof(struct key_value_entry) * meta_data->maps_block_length + meta_data->strings_block_size + (length + 1) + 200);
        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
        meta_data->strings_block_size += (length + 1) + 200;
    }

    *stringdex = meta_data->next_string_index;

    for (int i = 0; i < length + 1; i++) {
        strings[meta_data->next_string_index] = string[i];
        meta_data->next_string_index++;
    }

    return key_value_map;
}

int key_value_get_integer(void* key_value_map, char* name, int default_value, int* buffer) {

    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    int map_index = 0;
    for (; map_index < meta_data->next_map_index && strcmp(name, &strings[maps[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->next_map_index) {
        if (maps[map_index].type == _VALUE_TYPE_INTEGER) {
            *buffer = maps[map_index].value.integer;
            return 0;
        }
        else {
            maps[map_index].type = _VALUE_TYPE_INTEGER;
            maps[map_index].value.integer = default_value;
            *buffer = default_value;
            return 2;
        }

    }
    else {
        if (meta_data->next_map_index == meta_data->maps_block_length) {
            key_value_map = _key_value_expand_entries(key_value_map, 20);
            meta_data = key_value_map;
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
        }
        int key_index;
        key_value_map = _key_value_add_string(key_value_map, name, &key_index);
        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

        maps[map_index].key = key_index;
        maps[map_index].type = _VALUE_TYPE_INTEGER;
        maps[map_index].value.integer = default_value;
        *buffer = default_value;

        meta_data->next_map_index++;
        return 1;
    }

}
int key_value_get_float(void* key_value_map, char* name, float default_value, float* buffer) {
    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    int map_index = 0;
    for (; map_index < meta_data->next_map_index && strcmp(name, &strings[maps[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->next_map_index) {
        if (maps[map_index].type == _VALUE_TYPE_FLOAT) {
            *buffer = maps[map_index].value.floating;
            return 0;
        }
        else {
            maps[map_index].type = _VALUE_TYPE_FLOAT;
            maps[map_index].value.floating = default_value;
            *buffer = default_value;
            return 2;
        }

    }
    else {
        if (meta_data->next_map_index == meta_data->maps_block_length) {
            key_value_map = _key_value_expand_entries(key_value_map, 20);
            meta_data = key_value_map;
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
        }
        int key_index;
        key_value_map = _key_value_add_string(key_value_map, name, &key_index);
        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

        maps[map_index].key = key_index;
        maps[map_index].type = _VALUE_TYPE_FLOAT;
        maps[map_index].value.floating = default_value;
        *buffer = default_value;

        meta_data->next_map_index++;
        return 1;
    }
}


int key_value_get_string(void* key_value_map, char* name, char* default_value, char* buffer, int buffer_size) {
    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    int map_index = 0;
    for (; map_index < meta_data->next_map_index && strcmp(name, &strings[maps[map_index].key]) != 0; map_index++);

    if (map_index < meta_data->next_map_index) {
        if (maps[map_index].type == _VALUE_TYPE_STRING) {

            int string_length = strlen(strings[maps[map_index].value.string]) + 1;
            if (buffer_size < string_length) return 3;
            memcpy(buffer, &strings[maps[map_index].value.string], string_length);
            return 0;
        }
        else {
            maps[map_index].type = _VALUE_TYPE_STRING;
            int value_index;
            key_value_map = _key_value_add_string(key_value_map, default_value, &value_index);

            meta_data = key_value_map;
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

            maps[map_index].value.string = value_index;

            int string_length = strlen(&strings[maps[map_index].value.string]) + 1;
            if (buffer_size < string_length) return 3;
            memcpy(buffer, &strings[maps[map_index].value.string], string_length);
            return 2;
        }

    }
    else {
        if (meta_data->next_map_index == meta_data->maps_block_length) {
            key_value_map = _key_value_expand_entries(key_value_map, 20);
            meta_data = key_value_map;
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
        }
        int key_index;
        key_value_map = _key_value_add_string(key_value_map, name, &key_index);
        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

        maps[map_index].key = key_index;

        maps[map_index].type = _VALUE_TYPE_STRING;
        int value_index;
        key_value_map = _key_value_add_string(key_value_map, default_value, &value_index);

        meta_data = key_value_map;
        maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
        strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

        maps[map_index].value.string = value_index;

        int string_length = strlen(&strings[maps[map_index].value.string]) + 1;
        if (buffer_size < string_length) return 3;
        memcpy(buffer, &strings[maps[map_index].value.string], string_length);

        meta_data->next_map_index++;
        return 1;
    }
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

    int maps_block_length = 10;
    int next_map_index = 0;
    int strings_block_size = 200;
    int next_string_index = 0;

    char* key_value_map = malloc(sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry) + strings_block_size);

    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry);

    int lines = 1;

    for (int i = 0; i < fileSize; i++) if (buffer[i] == '\n') lines++;

    int text_i = 0;
    for (int lines_i = 0; lines_i < lines; lines_i++) {
#define skip_spaces while(buffer[text_i] == ' ') text_i++;
#define token_length(value) for(int _i = 0; buffer[text_i + _i] != ' ' && buffer[text_i+ _i] != ':' && buffer[text_i + _i] != '\n' && buffer[text_i + _i] != '\0'; _i++) value++;

#define add_string(length, string_index) \
        if(strings_block_size - next_string_index < length + 1) {\
            key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + sizeof(struct key_value_entry) * maps_block_length + strings_block_size + (length + 1) + 200);\
            strings_block_size = strings_block_size + (length + 1) + 200;\
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);\
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry);\
        }\
        string_index = next_string_index;\
        for (int _i = 0; _i < length; _i++) { strings[next_string_index] = buffer[text_i]; next_string_index++; text_i++; }\
        strings[next_string_index] = '\0'; next_string_index++;

        skip_spaces

        if (buffer[text_i] == '\n' || buffer[text_i] == '\0') { text_i++;  continue; }

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

        if (maps_block_length == next_map_index) {

            key_value_map = realloc(key_value_map, sizeof(struct key_value_meta_data) + (maps_block_length + 20) * sizeof(struct key_value_entry) + strings_block_size);
            maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);
            maps_block_length += 20;
            strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry);
            memmove(strings, strings - 20 * sizeof(struct key_value_entry), strings_block_size);
            
        }

        maps[next_map_index] = (struct key_value_entry){
            type,
            key_index,
            value,
        };

        next_map_index++;

    }

    struct key_value_meta_data* meta_data = key_value_map;
    meta_data->maps_block_length = maps_block_length;
    meta_data->next_map_index = next_map_index;
    meta_data->strings_block_size = strings_block_size;
    meta_data->next_string_index = next_string_index;

    free(buffer);
    return key_value_map;

_error:
    free(buffer);
    free(key_value_map);

    printf("error parsing yaml %s\n", file_path);

    return NULL;
}

void key_value_write_yaml(char* file_path, void* key_value_map) {
    
    struct key_value_meta_data* meta_data = key_value_map;
    struct key_value_entry* maps = (size_t)key_value_map + sizeof(struct key_value_meta_data);

    int text_buffer_size = 1000;
    char* text_buffer = malloc(text_buffer_size);

    char* strings = (size_t)key_value_map + meta_data->maps_block_length * sizeof(struct key_value_entry) + sizeof(struct key_value_meta_data);
    int text_i = 0;

    for (int map_i = 0; map_i < meta_data->next_map_index; map_i++) {
        
        {
            int _string_length = strlen(&strings[maps[map_i].key]);
            if (_string_length > text_buffer_size - text_i - 30) {
                text_buffer_size += _string_length + 200;
                text_buffer = realloc(text_buffer, text_buffer_size);
            }
            for (int _i = 0; _i < _string_length; _i++) { text_buffer[text_i] = strings[maps[map_i].key + _i]; text_i++; }
        }

        text_buffer[text_i] = ':';
        text_i++;
        text_buffer[text_i] = ' ';
        text_i++;

        if (maps[map_i].type == _VALUE_TYPE_INTEGER) {
            int divisor_max = 1;
            for (; maps[map_i].value.integer / divisor_max >= 10; divisor_max *= 10);
            for (; divisor_max > 0; divisor_max /= 10) {
                text_buffer[text_i] = '0' + (maps[map_i].value.integer / divisor_max) % 10; text_i++;
            }
        }
        else if (maps[map_i].type == _VALUE_TYPE_FLOAT) {
            int divisor_max = 1;
            int int_part = maps[map_i].value.floating;
            for (; int_part / divisor_max >= 10; divisor_max *= 10);
            for (; divisor_max > 0; divisor_max /= 10) {
                text_buffer[text_i] = '0' + (int_part / divisor_max) % 10; text_i++;
            }
            text_buffer[text_i] = '.';
            text_i++;
            float float_part = maps[map_i].value.floating - (float)int_part;

            int fractor = 10;
            for (; ((float)(int)(float_part * (float)fractor)) - (float_part * (float)fractor) != 0; fractor *= 10) {
                text_buffer[text_i] = '0' + (int)(float_part * (float)fractor) % 10; text_i++;
            }

        }
        else {
            text_buffer[text_i] = '"';
            text_i++;

            int string_length = strlen(&strings[maps[map_i].value.string]);
            if (string_length > text_buffer_size - text_i - 30) {
                text_buffer_size += string_length + 200;
                text_buffer = realloc(text_buffer, text_buffer_size);
            }
            for (int _i = 0; _i < string_length; _i++) {
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

    fwrite(text_buffer, sizeof(char), text_i, file);
    fclose(file);

}