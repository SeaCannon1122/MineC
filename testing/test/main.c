#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "string_allocator.h"
#include "hashmap.h"

int main(int argc, char* argv[]) {

	uint8_t* strings[] = { "Hello", ", ", "is", " ", "it", " ", "me", " ", "you're", " ", "looking", " ", "for", "?" };
	
	void* allocator = string_allocator_new(4096);

	printf(string_allocate_joined_string(allocator, strings, sizeof(strings) / sizeof(strings[0])));

	string_allocator_delete(allocator);

	printf("test");

	uint8_t* yaml_data = "";

	void* map = hashmap_new(4, 1);

	hashmap_read_yaml(map, yaml_data, strlen(yaml_data));

	struct hashmap_iterator it;
	hashmap_iterator_start(&it, map);

	struct hashmap_multi_type* val;
	uint8_t* key;

	while (val = hashmap_iterator_next_key_value_pair(&it, &key))
	{
		switch (val->type)
		{
		case HASHMAP_VALUE_STRING: {
			printf("key: %s   value: %s\n", key, val->data._string);
		} break;

		case HASHMAP_VALUE_INT: {
			printf("key: %s   value: %d\n", key, val->data._int);
		} break;

		case HASHMAP_VALUE_FLOAT: {
			printf("key: %s   value: %f\n", key, val->data._float);
		} break;
		}
		
	}

	hashmap_delete_key(map, "pi");
		
	hashmap_iterator_start(&it, map);

	while (val = hashmap_iterator_next_key_value_pair(&it, &key))
	{
		switch (val->type)
		{
		case HASHMAP_VALUE_STRING: {
			printf("key: %s   value: %s\n", key, val->data._string);
		} break;

		case HASHMAP_VALUE_INT: {
			printf("key: %s   value: %d\n", key, val->data._int);
		} break;

		case HASHMAP_VALUE_FLOAT: {
			printf("key: %s   value: %f\n", key, val->data._float);
		} break;
		}

	}

	hashmap_delete(map);

	return 0;
}