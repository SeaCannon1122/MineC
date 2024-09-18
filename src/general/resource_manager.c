#include "resource_manager.h"

#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>

#include "keyvalue.h"
#include "resource_loader.h"
#include "utils.h"

int rm_compare_resource_manager_entrys(const void* a, const void* b) {
	return strcmp(((struct key_value_map_entry*)a)->key, ((struct key_value_map_entry*)b)->key);
}


struct key_value_map* new_resource_manager(char* src_keyvalue) {
	int value_key_value_map = 99;

	char* data = load_text_file(src_keyvalue);
	if (data == NULL) return NULL;
	int pairs_count = 0;

	for (int i = 0; data[i] != '\0'; i++) if (data[i] == '\n') pairs_count++;

	struct key_value_map* map = malloc(sizeof(struct key_value_map) + sizeof(struct key_value_map_entry) * pairs_count);
	struct key_value_map_entry* mappings = (struct key_value_map_entry*)((long long)map + sizeof(struct key_value_map));

	map->mappings_count = pairs_count;
	map->mappings = mappings;

	int total_mappings_count = pairs_count;

	int line_begin_index = 0;

	for (int i = 0; i < pairs_count; i++) {
		int j = line_begin_index;

		for (; data[j] != ' '; j++) mappings[i].key[j - line_begin_index] = data[j];
		mappings[i].key[j - line_begin_index] = '\0';
		j += 3;
		int value_start = j;

		char temp_value[MAX_VALUE_SIZE];

		for (; data[j] != '\n'; j++) temp_value[j - value_start] = data[j];
		temp_value[j - value_start] = '\0';

		if (data[value_start - 2] == 'i') {
			mappings[i].value.i = string_to_int(temp_value, j - value_start);
			mappings[i].value_type = VALUE_INT;
		}
		else if (data[value_start - 2] == 'f') {
			mappings[i].value.f = string_to_float(temp_value, j - value_start);
			mappings[i].value_type = VALUE_FLOAT;
		}
		else if (data[value_start - 2] == 'r') {
			char file_path[1024];

			int split_index = 0;
			for (; src_keyvalue[split_index] != '\0'; split_index++);
			for (; src_keyvalue[split_index] != '/' && split_index > 0; split_index--);
			if (split_index != 0) split_index++;

			int k = 0;
			for (; k < split_index; k++) file_path[k] = src_keyvalue[k];
			for (k = 0; temp_value[k] != '\0'; k++) file_path[split_index + k] = temp_value[k];
			file_path[split_index + k] = '\0';

			struct key_value_map* sub_map = new_resource_manager(file_path);
			mappings[i].value.ptr = sub_map;
			mappings[i].value_type = value_key_value_map;

			total_mappings_count += (sub_map->mappings_count - 1);
		}
		else if (data[value_start - 2] == 'l') {

			char file_extension[64];
			{
				int k;
				for (k = 0; temp_value[k] != '.'; k++);
				k++;
				int extension_start = k;
				for (; temp_value[k] != '\0'; k++) file_extension[k - extension_start] = temp_value[k];
				file_extension[k - extension_start] = temp_value[k];
			}
			char file_path[1024];
			{
				int split_index = 0;
				for (; src_keyvalue[split_index] != '\0'; split_index++);
				for (; src_keyvalue[split_index] != '/' && split_index > 0; split_index--);
				if (split_index != 0) split_index++;

				int k = 0;
				for (; k < split_index; k++) file_path[k] = src_keyvalue[k];
				for (k = 0; temp_value[k] != '\0'; k++) file_path[split_index + k] = temp_value[k];
				file_path[split_index + k] = '\0';
			}

			if (file_extension[0] == 'p' && file_extension[1] == 'n' && file_extension[2] == 'g' && file_extension[3] == '\0') {
				mappings[i].value.ptr = load_argb_image_from_png(file_path);
			}
			else if (file_extension[0] == 'p' && file_extension[1] == 'i' && file_extension[2] == 'x' && file_extension[3] == 'e' && file_extension[4] == 'l' && file_extension[5] == 'f' && file_extension[6] == 'o' && file_extension[7] == 'n' && file_extension[8] == 't' && file_extension[9] == '\0') {
				mappings[i].value.ptr = load_char_font(file_path);
			}
			else if (file_extension[0] == 'k' && file_extension[1] == 'e' && file_extension[2] == 'y' && file_extension[3] == 'v' && file_extension[4] == 'a' && file_extension[5] == 'l' && file_extension[6] == 'u' && file_extension[7] == 'e' && file_extension[8] == '\0') {
				mappings[i].value.ptr = load_key_value_map(file_path);
			}
			else {
				mappings[i].value.ptr = load_text_file(file_path);
			}
			mappings[i].value_type = VALUE_PTR;
		}
		else {
			for (int k = 0; k < j - value_start + 1; k++) mappings[i].value.s[k] = temp_value[k];
			mappings[i].value_type = VALUE_STRING;
		}

		line_begin_index = j + 1;
	}

	free(data);

	struct key_value_map* combind_map = malloc(sizeof(struct key_value_map) + sizeof(struct key_value_map_entry) * total_mappings_count);
	struct key_value_map_entry* combind_map_mappings = (struct key_value_map_entry*)((long long)combind_map + sizeof(struct key_value_map));
	combind_map->mappings_count = total_mappings_count;
	combind_map->mappings = combind_map_mappings;

	int combind_map_index = 0;
	for (int i = 0; i < pairs_count; i++) {

		if (map->mappings[i].value_type == value_key_value_map) {
			struct key_value_map* sub_map = map->mappings[i].value.ptr;

			for (int k = 0; k < sub_map->mappings_count; k++) {

				for (int j = 0; j < MAX_KEY_SIZE; j++) combind_map->mappings[combind_map_index].key[j] = sub_map->mappings[k].key[j];
				for (int j = 0; j < MAX_VALUE_SIZE; j++) combind_map->mappings[combind_map_index].value.s[j] = sub_map->mappings[k].value.s[j];
				combind_map->mappings[combind_map_index].value_type = sub_map->mappings[k].value_type;
				combind_map_index++;
			}

			free(sub_map);

		}
		else {
			for (int j = 0; j < MAX_KEY_SIZE; j++) combind_map->mappings[combind_map_index].key[j] = map->mappings[i].key[j];
			for (int j = 0; j < MAX_VALUE_SIZE; j++) combind_map->mappings[combind_map_index].value.s[j] = map->mappings[i].value.s[j];
			combind_map->mappings[combind_map_index].value_type = map->mappings[i].value_type;
			combind_map_index++;
		}
	}

	free(map);

	qsort(combind_map->mappings, combind_map->mappings_count, sizeof(struct key_value_map_entry), rm_compare_resource_manager_entrys);
	return combind_map;
}

void destroy_resource_manager(struct key_value_map* manager) {
	for (int i = 0; i < manager->mappings_count; i++) {
		free(manager->mappings[i].value.ptr);
	}
	free(manager);
}