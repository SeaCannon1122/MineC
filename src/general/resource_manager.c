#include "resource_manager.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <STB_IMAGE/stb_image.h>

#include "general/rendering/vulkan_helpers.h"

void* layout_maps[32];
uint8_t layout_maps_paths[32][512];
uint32_t layout_maps_length;

uint32_t image_file_count;
uint32_t key_value_file_count;
uint32_t audio_file_count;
uint32_t shader_file_count;
uint32_t pixelfont_file_count;

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
	
		if (load_return_type == KEY_VALUE_ERROR_COULDNT_OPEN_FILE) printf("[RESOURCE MANAGER] Couldn't open resource layout file ");
		else if (load_return_type == KEY_VALUE_ERROR_FILE_INVALID_SYNTAX) printf("[RESOURCE MANAGER] Invalid syntax in resource layout file ");

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
		if (dot1 == -1) continue;

		

		int32_t dot2 = -1;
		for (; value >= 0; value--) {
			if (pair.value.string[value] == '/') break;
			if (pair.value.string[value] == '.') {
				dot2 = value;
				value++;
				break;
			}
		}

		if (dot1 - dot2 != sizeof("resourcelayout")) {

			if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) image_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) key_value_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) audio_file_count++;
			else if (
				strcmp(&pair.value.string[dot1 + 1], "spv") == 0 || 
				strcmp(&pair.value.string[dot1 + 1], "vert") == 0 || 
				strcmp(&pair.value.string[dot1 + 1], "frag") == 0 ||
				strcmp(&pair.value.string[dot1 + 1], "glsl") == 0
				) shader_file_count++;
			else if (strcmp(&pair.value.string[dot1 + 1], "pixelfont") == 0) pixelfont_file_count++;

			continue;
		}

		int32_t resource_layout = 1;

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

	return 0;
}

uint32_t resource_manager_new(struct resource_manager* rm, uint8_t* file_path) {

	layout_maps_length = 0;

	image_file_count = 0;
	key_value_file_count = 0;
	audio_file_count = 0;
	shader_file_count = 0;
	pixelfont_file_count = 0;

	_load_resource_layout(file_path);

	rm->image_count = 0;
	rm->images = malloc(sizeof(struct resource_manager_image) * image_file_count);
	rm->images_name_map = key_value_new(image_file_count + 5, image_file_count * 20);

	rm->shader_count = 0;
	rm->shaders = malloc(sizeof(struct resource_manager_image) * shader_file_count);
	rm->shaders_names_map = key_value_new(shader_file_count + 5, shader_file_count * 20);

	rm->pixelfont_count = 0;
	rm->pixelfonts = malloc(sizeof(struct pixel_font*) * pixelfont_file_count);
	rm->pixelfonts_names_map = key_value_new(pixelfont_file_count + 5, pixelfont_file_count * 20);

	rm->key_value_count = 0;
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
			if (dot1 == -1) continue;
			

			if (strcmp(&pair.value.string[dot1 + 1], "png") == 0) {
				uint32_t comp;

				rm->images[rm->image_count].data = stbi_load(sub_path, &rm->images[rm->image_count].width, &rm->images[rm->image_count].height, &comp, 4);

				if (rm->images[rm->image_count].data == NULL) printf("[RESOURCE MANAGER] Couldn't load %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				else {
					key_value_set_integer(&rm->images_name_map, pair.key, rm->image_count);

					rm->image_count++;
				}
			}


			else if (strcmp(&pair.value.string[dot1 + 1], "yaml") == 0) {

			}
			else if (strcmp(&pair.value.string[dot1 + 1], "mp3") == 0) {

			}
			else if (
				strcmp(&pair.value.string[dot1 + 1], "spv") == 0 || 
				strcmp(&pair.value.string[dot1 + 1], "vert") == 0 || 
				strcmp(&pair.value.string[dot1 + 1], "frag") == 0 || 
				strcmp(&pair.value.string[dot1 + 1], "glsl") == 0 
			) {

				rm->shaders[rm->shader_count].source_data = _resource_manager_load_file(sub_path, &rm->shaders[rm->shader_count].source_data_size);

				if (rm->shaders[rm->shader_count].source_data == NULL) printf("[RESOURCE MANAGER] Couldn't load %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				else {
					key_value_set_integer(&rm->shaders_names_map, pair.key, rm->shader_count);

					rm->shader_count++;
				}
			}

			else if (strcmp(&pair.value.string[dot1 + 1], "pixelfont") == 0) {

				rm->pixelfonts[rm->pixelfont_count] = load_pixel_font(sub_path);

				if (rm->pixelfonts[rm->pixelfont_count] == NULL) printf("[RESOURCE MANAGER] Couldn't load %s from %s\n", sub_path, &layout_maps_paths[i][0]);

				else {
					key_value_set_integer(&rm->pixelfonts_names_map, pair.key, rm->pixelfont_count);

					rm->pixelfont_count++;
				}
			}

		}

	}

	return 0;
}



uint32_t resource_manager_destroy(struct resource_manager* rm) {

	for (uint32_t i = 0; i < rm->image_count; i++) free(rm->images[i].data);
	free(rm->images_name_map);
	free(rm->images);

	for (uint32_t i = 0; i < rm->shader_count; i++) free(rm->shaders[i].source_data);
	free(rm->shaders_names_map);
	free(rm->shaders);

	for (uint32_t i = 0; i < rm->pixelfont_count; i++) free(rm->pixelfonts[i]);
	free(rm->pixelfonts_names_map);
	free(rm->pixelfonts);

	return 0;
}

uint32_t resource_manager_get_image(struct resource_manager* rm, uint8_t* name, struct resource_manager_image* image) {

	uint64_t image_index;
	key_value_get_integer(&rm->images_name_map, name, -1, &image_index);

	if (image_index == -1) return 1;

	*image = rm->images[image_index];

	return 0;
}


uint32_t resource_manager_get_shader(struct resource_manager* rm, uint8_t* name, struct resource_manager_shader* shader) {

	int64_t shader_index;
	key_value_get_integer(&rm->shaders_names_map, name, -1, &shader_index);
	
	if (shader_index == -1) return 1;

	*shader = rm->shaders[shader_index];

	return 0;
}

uint32_t resource_manager_get_pixelfont(struct resource_manager* rm, uint8_t* name, struct pixel_font** pixelfont) {

	int64_t pixelfont_index;
	key_value_get_integer(&rm->pixelfonts_names_map, name, -1, &pixelfont_index);

	if (pixelfont_index == -1) return 1;

	*pixelfont = rm->pixelfonts[pixelfont_index];

	return 0;
}