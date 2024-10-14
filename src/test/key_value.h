#pragma once

struct key_value_meta_data {
	int maps_block_length;
	int next_map_index;
	int strings_block_size;
	int next_string_index;
};

struct key_value_entry {
	int type;
	int key;
	union {
		long long integer;
		float floating;
		int string;
	} value;
};

int key_value_get_integer(void* key_value_map, char* name, int default_value, int* buffer);
int key_value_get_float(void* key_value_map, char* name, float default_value, float* buffer);
int key_value_get_string(void* key_value_map, char* name, char* default_value, char* buffer, int buffer_size);


void* key_value_load_yaml(char* file_path);
void key_value_write_yaml(char* file_path, void* key_value_map);