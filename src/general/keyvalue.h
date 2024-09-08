#pragma once

#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 128

enum VALUE_TYPE {
	VALUE_STRING = 0,
	VALUE_INT = 1,
	VALUE_FLOAT = 2,
	VALUE_PTR = 3
};

union multi_type {
	int i;
	float f;
	void* ptr;
};

struct key_value_map_entry {
	char key[MAX_KEY_SIZE];
	union {
		char s[MAX_VALUE_SIZE];
		int i;
		float f;
		void* ptr;
	} value;
	char value_type;
};

struct key_value_map {
	struct key_value_map_entry* mappings;
	int mappings_count;
};

union multi_type get_value_from_key(struct key_value_map* manager, char* key);