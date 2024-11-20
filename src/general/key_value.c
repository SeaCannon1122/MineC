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
    strings = (size_t)key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
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

int key_value_set_integer(void** key_value_map, char* name, long long value) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {
        maps[map_index].value.integer = value;

        if (maps[map_index].type == _VALUE_TYPE_INTEGER) return 0;
        else {
            maps[map_index].type = _VALUE_TYPE_INTEGER;
            return 2;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, 20);
    }
    int key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = _VALUE_TYPE_INTEGER;
    maps[meta_data->next_map_index].value.integer = value;

    meta_data->next_map_index++;
    return 1;
}

int key_value_set_float(void** key_value_map, char* name, float value) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {
        maps[map_index].value.floating = value;

        if (maps[map_index].type == _VALUE_TYPE_FLOAT) return 0;
        else {
            maps[map_index].type = _VALUE_TYPE_FLOAT;
            return 2;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, 20);
    }
    int key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = _VALUE_TYPE_FLOAT;
    maps[meta_data->next_map_index].value.floating = value;

    meta_data->next_map_index++;
    return 1;
}

int key_value_set_string(void** key_value_map, char* name, char* value) {
    int value_index;
    *key_value_map = _key_value_add_string(*key_value_map, value, &value_index);

    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0) {      
        maps[map_index].value.string = value_index;

        if (maps[map_index].type == _VALUE_TYPE_STRING) return 0;
        else {
            maps[map_index].type = _VALUE_TYPE_STRING;
            return 2;
        }
    }

    if (meta_data->next_map_index == meta_data->maps_block_length) {
        *key_value_map = _key_value_expand_entries(*key_value_map, 20);
    }
    int key_index;
    *key_value_map = _key_value_add_string(*key_value_map, name, &key_index);
    meta_data = *key_value_map;
    maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    maps[meta_data->next_map_index].key = key_index;
    maps[meta_data->next_map_index].type = _VALUE_TYPE_STRING;
    maps[meta_data->next_map_index].value.string = value_index;

    meta_data->next_map_index++;
    return 1;
}

int key_value_get_integer(void** key_value_map, char* name, long long default_value, long long* buffer) {

    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == _VALUE_TYPE_INTEGER) {
        *buffer = maps[map_index].value.integer;
        return 0;
    }

    *buffer = default_value;
    return key_value_set_integer(key_value_map, name, default_value);
}
int key_value_get_float(void** key_value_map, char* name, float default_value, float* buffer) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);
    
    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == _VALUE_TYPE_FLOAT) {
        *buffer = maps[map_index].value.floating;
        return 0;
    }

    *buffer = default_value;
    return key_value_set_float(key_value_map, name, default_value);
}


int key_value_get_string(void** key_value_map, char* name, char* default_value, char* buffer, int buffer_size) {
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);
    char* strings = (size_t)*key_value_map + sizeof(struct key_value_meta_data) + meta_data->maps_block_length * sizeof(struct key_value_entry);

    for (int map_index = 0; map_index < meta_data->next_map_index; map_index++) if (strcmp(name, &strings[maps[map_index].key]) == 0 && maps[map_index].type == _VALUE_TYPE_STRING) {
        int string_length = strlen(&strings[maps[map_index].value.string]) + 1;
        if (buffer_size < string_length) return 3;
        memcpy(buffer, &strings[maps[map_index].value.string], string_length);
        return 0;
    }

    int string_length = strlen(default_value) + 1;
    if (buffer_size < string_length) return 3;
    memcpy(buffer, default_value, string_length);

    return key_value_set_string(key_value_map, name, default_value);
}

void* key_value_new(int maps_block_length, int strings_block_size) {

    struct key_value_meta_data* map = malloc(sizeof(struct key_value_meta_data) + maps_block_length * sizeof(struct key_value_entry) + strings_block_size);

    map->maps_block_length = maps_block_length;
    map->next_map_index = 0;
    map->strings_block_size = strings_block_size;
    map->next_string_index = 0;

    return map;
}

int key_value_load_yaml(void** key_value_map, char* file_path) {
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

    int text_i = 0;
    for (int lines_i = 0; lines_i < line_breaks + 1; lines_i++) {
#define skip_spaces while(buffer[text_i] == ' ') text_i++;    

        skip_spaces
        if (buffer[text_i] == '\n' || buffer[text_i] == '\0') { text_i++;  continue; }

        int key_start = text_i;
        for (; buffer[text_i] != ' ' && buffer[text_i] != ':' && buffer[text_i] != '\n' && buffer[text_i] != '\0'; text_i++);
        int key_end = text_i;
        if (key_end - key_start == 0) goto _error;
        
        skip_spaces
        if (buffer[text_i] != ':') goto _error;
        text_i++;
        buffer[key_end] = '\0';

        skip_spaces

        int type = _VALUE_TYPE_INTEGER;
        int string_quotes = 0;

        int value_start = text_i;

        for (; buffer[text_i] != '\n' && buffer[text_i ] != '\0' && (buffer[text_i] != ' ' || string_quotes == 1); text_i++) {
            if (buffer[text_i] == '"') { type = _VALUE_TYPE_STRING; string_quotes ^= 1; }
            if (type == _VALUE_TYPE_STRING) continue;
            if (buffer[text_i] == '.' && type == _VALUE_TYPE_INTEGER) type = _VALUE_TYPE_FLOAT;
            else if ((buffer[text_i] < '0' || buffer[text_i] > '9') && (buffer[text_i] != '-' || text_i != value_start)) { type = _VALUE_TYPE_STRING; }
        }

        if (type == _VALUE_TYPE_STRING) {
            if (buffer[text_i - 1] == '"' && string_quotes == 0) {
                buffer[text_i - 1] = '\0';
                key_value_set_string(key_value_map, &buffer[key_start], &buffer[value_start + 1]);
            }
            else {
                buffer[text_i] = '\0';
                key_value_set_string(key_value_map, &buffer[key_start], &buffer[value_start]);
            }
        }
        else if (type == _VALUE_TYPE_INTEGER) {
            long long int_val = 0;
            int negative = 1;

            if (buffer[value_start] == '-') { value_start++; negative = -1; }
            for (int _i = value_start; _i < text_i; _i++) {
                int_val *= 10;
                int_val += buffer[_i] - '0';
            }
            int_val *= negative;
            key_value_set_integer(key_value_map, &buffer[key_start], int_val);
        }
        else {
            float float_val = 0;
            int decimal = -1;
            float negative = 1.f;

            if (buffer[value_start] == '-') { value_start++; negative = -1.f; }

            for (int _i = value_start; _i < text_i; _i++) {
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
            key_value_set_integer(key_value_map, &buffer[key_start], float_val);
        }

        skip_spaces

        if (buffer[text_i] != '\n' && buffer[text_i] != '\0') goto _error;

        text_i++;

    }

    free(buffer);
    return 0;

_error:
    free(buffer);

    printf("error parsing yaml %s\n", file_path);

    return 1;
}

void key_value_write_yaml(void** key_value_map, char* file_path) {
    
    struct key_value_meta_data* meta_data = *key_value_map;
    struct key_value_entry* maps = (size_t)*key_value_map + sizeof(struct key_value_meta_data);

    int text_buffer_size = 1000;
    char* text_buffer = malloc(text_buffer_size);

    char* strings = (size_t)*key_value_map + meta_data->maps_block_length * sizeof(struct key_value_entry) + sizeof(struct key_value_meta_data);
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