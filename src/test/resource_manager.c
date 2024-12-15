#include "resource_manager.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

void* layout_maps[32];
uint8_t layout_maps_paths[32][512];
uint32_t layout_maps_length;

uint32_t error;

uint32_t _load_resource_layout(uint8_t* file_path) {

	uint8_t sub_path[512];

	uint32_t layout_maps_index = layout_maps_length;

	sprintf(&layout_maps_paths[layout_maps_index][0], file_path);
	sprintf(sub_path, file_path);

	uint32_t sub_path_length = strlen(file_path);

	for (; file_path[sub_path_length] != '/'; sub_path_length--) {
		sub_path[sub_path_length] = 0;
	}
	sub_path_length++;

	layout_maps[layout_maps_index] = key_value_new(100, 1000);
	layout_maps_length++;

	if (key_value_load_yaml(&layout_maps[layout_maps_index], file_path) & KEY_VALUE_ERROR_MASK) { error = 1;  return layout_maps_index; }

	uint32_t map_size = key_value_get_size(layout_maps[layout_maps_index]);

	for (uint32_t i = 0; i < map_size; i++) {

		struct key_value_pair pair;
		key_value_get_pair(layout_maps[layout_maps_index], &pair, i);

		uint32_t value = strlen(pair.value.string);

		uint32_t dot1 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot1 = value;
				value--;
				break;
			}
		}
		if (dot1 == -1) continue;

		uint32_t dot2 = -1;
		for (; value >= 0 && pair.value.string[value] != '/'; value--) {
			if (pair.value.string[value] == '.') {
				dot2 = value;
				value++;
				break;
			}
		}
		if (dot2 == -1) continue;

		if (dot1 - dot2 != sizeof("resourcelayout")) continue;

		uint32_t resource_layout = 1;

		for (; value < dot1; value++) {
			if ("resourcelayout"[value - dot2 - 1] != pair.value.string[value]) {
				resource_layout = 0;
				break;
			}

		}

		if (resource_layout == 0) continue;

		sprintf(&sub_path[sub_path_length], pair.value.string);

		_load_resource_layout(sub_path);


	}


}

void* resource_manager_new(uint8_t* file_path) {

	layout_maps_length = 0;
	error = 0;

	_load_resource_layout(file_path);

	printf("test");

	

}