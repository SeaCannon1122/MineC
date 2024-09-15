#include "keyvalue.h"

#include <string.h>

#include "resource_loader.h"


union multi_type get_value_from_key(struct key_value_map* manager, char* key) {

	int left = 0;
	int right = manager->mappings_count - 1;

	while (left <= right) {
		int mid = left + (right - left) / 2;

		int cmp_result = strcmp(manager->mappings[mid].key, key);

		if (cmp_result == 0) {
			if (manager->mappings[mid].value_type == VALUE_STRING) {
				char* str = manager->mappings[mid].value.s;
				return *(union multi_type*)&str;
			}
			else return *(union multi_type*)&manager->mappings[mid].value.ptr;
		}
		else if (cmp_result < 0) {
			left = mid + 1;
		}
		else {
			right = mid - 1;
		}
	}

	void* ret = 0;

	return *(union multi_type*)&ret;
}
