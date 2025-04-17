#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "hashmap.h"

int main(int argc, char* argv[]) {

	void* map = hashmap_new(2, 1);

	hashmap_set_string(map, "key0", "value0");
	hashmap_set_string(map, "key1", "value1");
	hashmap_set_string(map, "key2", "value2");
	hashmap_set_string(map, "key3", "value3");
	hashmap_set_string(map, "key4", "value4");
	float pi = 3.141;
	hashmap_set_data(map, "key5", &pi, sizeof(float));

	printf("%s\n", hashmap_get_string(map, "key0"));
	printf("%s\n", hashmap_get_string(map, "key1"));
	printf("%s\n", hashmap_get_string(map, "key2"));
	printf("%s\n", hashmap_get_string(map, "key3"));
	printf("%s\n", hashmap_get_string(map, "key4"));

	float maybe_pi;
	hashmap_get_data(map, "key5", &maybe_pi, sizeof(float));
	printf("%f\n", maybe_pi);

	hashmap_delete(map);

	return 0;
}