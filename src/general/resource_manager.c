#include "resource_manager.h"

#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>

#include "keyvalue.h"
#include "resource_loader.h"
#include "utils.h"



struct key_value_map* new_resource_manager(char* src_keyvalue) {

	struct key_value_map* kvm = load_key_value_map(src_keyvalue);

	char layout_folder_path_size = string_length(src_keyvalue) - 1;
	for (; src_keyvalue[layout_folder_path_size - 1] != '/'; layout_folder_path_size--);

	struct key_value_map* manager = malloc(sizeof(struct key_value_map_entry) * kvm->mappings_count + sizeof(struct key_value_map));
	manager->mappings = (size_t)manager + sizeof(struct key_value_map);
	manager->mappings_count = kvm->mappings_count;

	for (int i = 0; i < kvm->mappings_count; i++) {

		manager->mappings[i].value_type = VALUE_PTR;

		char* resource_path = alloca(layout_folder_path_size + MAX_VALUE_SIZE);

		int k = 0;
		for (; k < layout_folder_path_size; k++) resource_path[k] = src_keyvalue[k];
		for (; kvm->mappings[i].value.s[k - layout_folder_path_size] != '\0'; k++) resource_path[k] = kvm->mappings[i].value.s[k - layout_folder_path_size];
		resource_path[k] = '\0';

		int j = 0;
		for (; kvm->mappings[i].key[j] != '\0'; j++) manager->mappings[i].key[j] = kvm->mappings[i].key[j];
		manager->mappings[i].key[j] = kvm->mappings[i].key[j];

		char file_extension[64];
		for (j = 0; kvm->mappings[i].value.s[j] != '.'; j++);
		j++;
		int extension_start = j;

		for (; kvm->mappings[i].value.s[j] != '\0'; j++) file_extension[j-extension_start] = kvm->mappings[i].value.s[j];
		
		file_extension[j - extension_start] = kvm->mappings[i].value.s[j];

		if (file_extension[0] == 'p' && file_extension[1] == 'n' && file_extension[2] == 'g' && file_extension[3] == '\0') {
			manager->mappings[i].value.ptr = load_argb_image_from_png(resource_path);
		}
		else if (file_extension[0] == 'p' && file_extension[1] == 'i' && file_extension[2] == 'x' && file_extension[3] == 'e' && file_extension[4] == 'l' && file_extension[5] == 'f' && file_extension[6] == 'o' && file_extension[7] == 'n' && file_extension[8] == 't' && file_extension[9] == '\0') {
			manager->mappings[i].value.ptr = load_char_font(resource_path);
		}
		else if (file_extension[0] == 'k' && file_extension[1] == 'e' && file_extension[2] == 'y' && file_extension[3] == 'v' && file_extension[4] == 'a' && file_extension[5] == 'l' && file_extension[6] == 'u' && file_extension[7] == 'e' && file_extension[8] == '\0') {
			manager->mappings[i].value.ptr = load_key_value_map(resource_path);
		}
		else {
			manager->mappings[i].value.ptr = load_text_file(resource_path);
		}

	}

	free(kvm);

	return manager;
}

void destroy_resource_manager(struct key_value_map* manager) {
	for (int i = 0; i < manager->mappings_count; i++) free(manager->mappings[i].value.ptr);
	free(manager);
}