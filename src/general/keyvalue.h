#pragma once

#define MAX_KEY_SIZE 64
#define MAX_VALUE_SIZE 256

enum VALUE_TYPE {
	VALUE_STRING = 0,
	VALUE_INT = 1,
	VALUE_FLOAT = 2,
};

struct key_value_map_entry {
	char key[MAX_KEY_SIZE];
	union {
		char s[MAX_VALUE_SIZE];
		int i;
		float f;
	} value;
	char value_type;
};

struct key_value_map {
	struct key_value_map_entry* mappings;
	int mappings_count;
};