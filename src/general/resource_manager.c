#include "resource_manager.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <STB_IMAGE/stb_image.h>

void* layout_maps[32];
uint8_t layout_maps_paths[32][512];
uint32_t layout_maps_length;

uint32_t image_file_count;
uint32_t key_value_file_count;
uint32_t audio_file_count;
uint32_t binary_file_count;

int (*LOG)(const char* const Format, ...);

struct _resource_manager_key_value_map {
	void* map;
	uint32_t modifiable_bool;
};

uint8_t* _resource_manager_load_file(uint8_t* src, uint32_t* size) {

	FILE* file = fopen(src, "rb");
	if (file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	uint8_t* buffer = malloc(fileSize);

	size_t bytesRead = fread(buffer, 1, fileSize, file);

	fclose(file);

	*size = bytesRead;

	return buffer;
}

uint32_t _load_resource_layout(uint8_t* file_path) {

	uint8_t sub_path[512];

	uint32_t layout_maps_index = layout_maps_length;

	sprintf(&layout_maps_paths[layout_maps_index][0], file_path);

	layout_maps[layout_maps_index] = key_value_new(100, 1000);
	layout_maps_length++;
	enum key_value_return_type load_return_type = key_value_load_yaml(&layout_maps[layout_maps_index], file_path);

	if (load_return_type & KEY_VALUE_ERROR_MASK) { 
	
		if (load_return_type == KEY_VALUE_ERROR_COULDNT_OPEN_FILE) LOG("[RESOURCE MANAGER] Couldn't open resource layout file ");
		else if (load_return_type == KEY_VALUE_ERROR_FILE_INVALID_SYNTAX) LOG("[RESOURCE MANAGER] Invalid syntax in resource layout file ");

		printf("%s\n", file_path);

		return 0;
	}

	sprintf(sub_path, file_path);

	int32_t sub_path_length = strlen(file_path);

	for (; sub_path_length >= 0; sub_path_length--) {
		if (file_path[sub_path_length] == '/') break;
		sub_path[sub_path_length] = 0;
	}

	sub_path_length++;

	uint32_t map_size = key_value_get_size(layout_maps[layout_maps_index]);

	for (uint32_t i = 0; i < map_size; i++) {

		struct key_value_pair pair;
		key_value_get_pair(layout_maps[layout_maps_index], &pair, i);

		if (pair.type != VALUE_TYPE_STRING) continue;

		int32_t value = strlen(pair.value.string);

		uint32_t dot1 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot1 = value;
				value--;
				break;
			}
		}
		if (dot1 == -1) {
			binary_file_count++;
			continue;
		}

		int32_t dot2 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot2 = value;
				break;
			}
		}

		if (dot2 != -1) {
			value++;
			for (; pair.value.string[value] == "resourcelayout"[value - dot2 - 1]; value++);

			if (value == dot1) {
				sprintf(&sub_path[sub_path_length], pair.value.string);
				_load_resource_layout(sub_path);

				continue;
			}

		}

		if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) image_file_count++;
		else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) key_value_file_count++;
		else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) audio_file_count++;
		else binary_file_count++;

	}

	return 0;
}

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path, int (*log_function)(const char* const, ...)) {

	LOG = log_function;

	layout_maps_length = 0;

	image_file_count = 0;
	key_value_file_count = 0;
	audio_file_count = 0;
	binary_file_count = 0;

	_load_resource_layout(file_path);

	rm->image_count = 0;
	rm->images = malloc(sizeof(struct resource_manager_image) * image_file_count);
	rm->images_name_map = key_value_new(image_file_count + 5, image_file_count * 40);

	rm->binaries_count = 0;
	rm->binaries = malloc(sizeof(struct resource_manager_image) * binary_file_count);
	rm->binaries_names_map = key_value_new(binary_file_count + 5, binary_file_count * 40);

	rm->key_value_count = 0;
	rm->key_value_maps = malloc(sizeof(struct _resource_manager_key_value_map) * key_value_file_count);
	rm->key_value_maps_names_map = key_value_new(key_value_file_count + 5, key_value_file_count * 40);
	rm->key_value_maps_paths_map = key_value_new(key_value_file_count + 5, key_value_file_count * 40);

	rm->audio_count = 0;

	uint8_t sub_path[512];

	for (uint32_t i = 0; i < layout_maps_length; i++) {
		uint32_t size = key_value_get_size(layout_maps[i]);

		sprintf(sub_path, &layout_maps_paths[i][0]);

		int32_t sub_path_length = strlen(&layout_maps_paths[i][0]);

		for (; sub_path_length >= 0; sub_path_length--) {
			if (layout_maps_paths[i][sub_path_length] == '/') break;
			sub_path[sub_path_length] = 0;
		}

		sub_path_length++;

		for (uint32_t key_i = 0; key_i < size; key_i++) {

			struct key_value_pair pair;
			key_value_get_pair(layout_maps[i], &pair, key_i);

			if (pair.type != VALUE_TYPE_STRING) continue;

			int32_t value = strlen(pair.value.string);
			sprintf(&sub_path[sub_path_length], pair.value.string);

			uint32_t dot1 = -1;
			for (; value >= 0; value--) {
				if (pair.value.string[value] == '/') break;
				if (pair.value.string[value] == '.') {
					dot1 = value;
					value--;
					break;
				}
			}
			if (dot1 == -1) dot1 = strlen(pair.value.string) - 1;
			

			if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) {
				uint32_t comp;

				rm->images[rm->image_count].data = stbi_load(sub_path, &rm->images[rm->image_count].width, &rm->images[rm->image_count].height, &comp, 4);

				if (rm->images[rm->image_count].data == NULL) LOG("[RESOURCE MANAGER] Couldn't load %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				key_value_set_integer(&rm->images_name_map, pair.key, rm->image_count);
				rm->image_count++;
			}


			else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) {

				((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].modifiable_bool = 0;

				int32_t dot2 = -1;
				for (; value >= 0; value--) {
					if (pair.value.string[value] == '/') break;
					if (pair.value.string[value] == '.') {
						dot2 = value;
						break;
					}
				}

				if (dot2 != -1) {
					value++;
					for (; pair.value.string[value] == "resourcelayout"[value - dot2 - 1]; value++);
					if (value == dot1) continue;

					value = dot2 + 1;
					for (; pair.value.string[value] == "mod"[value - dot2 - 1]; value++);
					if (value == dot1) ((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].modifiable_bool = 1;

				}
				
				((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].map = key_value_new(20, 200);
	
				enum key_value_return_type load_return_type	= key_value_load_yaml(&((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].map, sub_path);

				if (load_return_type & KEY_VALUE_ERROR_MASK) {

					free(((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].map);
					((struct _resource_manager_key_value_map*)rm->key_value_maps)[rm->key_value_count].map = NULL;

					if (load_return_type == KEY_VALUE_ERROR_COULDNT_OPEN_FILE) LOG("[RESOURCE MANAGER] Couldn't open %s from %s\n", sub_path, &layout_maps_paths[i][0]);
					else if (load_return_type == KEY_VALUE_ERROR_FILE_INVALID_SYNTAX) LOG("[RESOURCE MANAGER] Invalid syntax in %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				}

				key_value_set_integer(&rm->key_value_maps_names_map, pair.key, rm->key_value_count);
				key_value_set_string(&rm->key_value_maps_paths_map, pair.key, sub_path);
				rm->key_value_count++;


			}
			else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) {

			}

			else {

				rm->binaries[rm->binaries_count].data = _resource_manager_load_file(sub_path, &rm->binaries[rm->binaries_count].size);

				if (rm->binaries[rm->binaries_count].data == NULL) LOG("[RESOURCE MANAGER] Couldn't load %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				key_value_set_integer(&rm->binaries_names_map, pair.key, rm->binaries_count);
				rm->binaries_count++;
			}

		}

	}

	return 0;
}



uint32_t resource_manager_destroy(struct resource_manager* rm) {

	for (uint32_t i = 0; i < rm->image_count; i++) free(rm->images[i].data);
	free(rm->images_name_map);
	free(rm->images);

	for (uint32_t i = 0; i < rm->binaries_count; i++) free(rm->binaries[i].data);
	free(rm->binaries_names_map);
	free(rm->binaries);

	for (uint32_t i = 0; i < rm->key_value_count; i++) free(((struct _resource_manager_key_value_map*)rm->key_value_maps)[i].map);
	free(rm->key_value_maps_names_map);
	free(rm->key_value_maps_paths_map);
	free(rm->key_value_maps);

	return 0;
}

uint32_t resource_manager_get_image(struct resource_manager* rm, uint8_t* name, struct resource_manager_image* image) {

	uint64_t image_index;
	key_value_get_integer(&rm->images_name_map, name, -1, &image_index);

	if (image_index == -1) return 1;
	if (rm->images[image_index].data == NULL) return 2;


	*image = rm->images[image_index];

	return 0;
}


uint32_t resource_manager_get_binary(struct resource_manager* rm, uint8_t* name, struct resource_manager_binary* binary) {

	int64_t binarie_index;
	key_value_get_integer(&rm->binaries_names_map, name, -1, &binarie_index);
	
	if (binarie_index == -1) return 1;
	if (rm->binaries[binarie_index].data == NULL) return 2;

	*binary = rm->binaries[binarie_index];

	return 0;
}


uint32_t resource_manager_get_key_value_map(struct resource_manager* rm, uint8_t* name, void** key_value_map) {

	int64_t key_value_index;
	key_value_get_integer(&rm->key_value_maps_names_map, name, -1, &key_value_index);

	if (key_value_index == -1) return 1;
	if (((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map == NULL) return 2;

	uint32_t size = key_value_get_size(((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map);

	*key_value_map = key_value_copy(((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map);

	return 0;
}

uint32_t resource_manager_save_key_value_map(struct resource_manager* rm, uint8_t* name, void* key_value_map) {

	int64_t key_value_index;
	key_value_get_integer(&rm->key_value_maps_names_map, name, -1, &key_value_index);

	if (key_value_index == -1) return 1;
	
	if (((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].modifiable_bool == 0) return 2;

	if (((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map != NULL) 
		free(((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map);

	((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map = key_value_copy(key_value_map);

	uint8_t path[256];
	if(key_value_get_string(&rm->key_value_maps_paths_map, name, " ", path, sizeof(path)) != KEY_VALUE_INFO_DEFAULT) return 2;

	key_value_write_yaml(&((struct _resource_manager_key_value_map*)rm->key_value_maps)[key_value_index].map, path);

	return 0;
}