#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "hashmap.h"

int main(int argc, char* argv[]) {

	void* map = hashmap_new(2, 1);

	hashmap_set_value(map, "key0", "value0", HASHMAP_VALUE_STRING);
	float pi = 3.141;
	hashmap_set_value(map, "key5", &pi, HASHMAP_VALUE_FLOAT);

	struct hashmap_multi_type* val;

	if (val = hashmap_get_value(map, "key0")) if (val->type == HASHMAP_VALUE_STRING) printf("%s\n", val->data._string);

	hashmap_delete(map);

	return 0;
}