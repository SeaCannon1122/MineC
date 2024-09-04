#include "resource_manager.h"

#include <stdlib.h>

#include "resource_loader.h"

void* resource_manager = NULL;
int resources_count = 0;

void resource_manager_init() {
#if defined(CLIENT_COMPILATION)
	char layout_file_path[] = "../../../resources/client/assets/resourcelayout.keyvalue";
	struct key_value_map* kvm = load_key_value_map(layout_file_path);

	char layout_folder_path[] = "../../../resources/client/assets/";
	int layout_folder_path_size = sizeof(layout_folder_path) - 1;
#elif defined(SERVER_COMPILATION)
	char layout_file_path[] = "../../../resources/server/assets/resourcelayout.keyvalue";
	struct key_value_map* kvm = load_key_value_map(layout_file_path);

	char layout_folder_path[] = "../../../resources/server/assets/";
	int layout_folder_path_size = sizeof(layout_folder_path) - 1;
#endif

	resources_count = kvm->mappings_count;

	resource_manager = malloc(sizeof(struct resource_manager_entry) * kvm->mappings_count);

	for (int i = 0; i < kvm->mappings_count; i++) {

		char resource_path[MAX_VALUE_SIZE];
		int k = 0;
		for (; k < layout_folder_path_size; k++) resource_path[k] = layout_folder_path[k];
		for (; kvm->mappings[i].value.s[k - layout_folder_path_size] != '\0'; k++) resource_path[k] = kvm->mappings[i].value.s[k - layout_folder_path_size];
		resource_path[k] = '\0';

		int j = 0;
		for (; kvm->mappings[i].key[j] != '\0'; j++) ((struct resource_manager_entry*)resource_manager)[i].resource_token[j] = kvm->mappings[i].key[j];
		((struct resource_manager_entry*)resource_manager)[i].resource_token[j] = kvm->mappings[i].key[j];

		char file_extension[64];
		for (j = 0; kvm->mappings[i].key[j] != '.'; j++);
		j++;
		int extension_start = j;
		for (; kvm->mappings[i].key[j] != '\0'; j++) file_extension[j-extension_start] = kvm->mappings[i].key[j];
		file_extension[j - extension_start] = kvm->mappings[i].key[j];

		if (file_extension[0] == 'p' && file_extension[1] == 'n' && file_extension[2] == 'g' && file_extension[3] == '\0') {
			((struct resource_manager_entry*)resource_manager)[i].resource_type = RESOURCE_ARGB_IMAGE;
			((struct resource_manager_entry*)resource_manager)[i].data = load_argb_image_from_png(resource_path);
		}
		else if (file_extension[0] == 'p' && file_extension[1] == 'i' && file_extension[2] == 'x' && file_extension[3] == 'e' && file_extension[4] == 'l' && file_extension[5] == 'f' && file_extension[6] == 'o' && file_extension[7] == 'n' && file_extension[8] == 't' && file_extension[9] == '\0') {
			((struct resource_manager_entry*)resource_manager)[i].resource_type = RESOURCE_PIXELFONT;
			((struct resource_manager_entry*)resource_manager)[i].data = load_char_font(resource_path);
		}
		else if (file_extension[0] == 'k' && file_extension[1] == 'e' && file_extension[2] == 'y' && file_extension[3] == 'v' && file_extension[4] == 'a' && file_extension[5] == 'l' && file_extension[6] == 'u' && file_extension[7] == 'e' && file_extension[8] == '\0') {
			((struct resource_manager_entry*)resource_manager)[i].resource_type = RESOURCE_KEYVALUE;
			((struct resource_manager_entry*)resource_manager)[i].data = load_key_value_map(resource_path);
		}
		else {
			((struct resource_manager_entry*)resource_manager)[i].resource_type = RESOURCE_TEXT_STRING;
			((struct resource_manager_entry*)resource_manager)[i].data = load_text_file(resource_path);
		}

	}

	free(kvm);

}

void* get_resource(char* token) {
	for (int i = 0; i < resources_count; i++) {
		int j = 0;
		for (;token[j] != '\0' && ((struct resource_manager_entry*)resource_manager)[i].resource_token[j] == token[j]; j++);
		
		if(((struct resource_manager_entry*)resource_manager)[i].resource_token[j] == '\0' && token[j] == '\0') {
			return ((struct resource_manager_entry*)resource_manager)[i].data;
		}

	}
	return NULL;
}

void resource_manager_exit() {
	for (int i = 0; i < resources_count; i++) free(((struct resource_manager_entry*)resource_manager)[i].data);
	free(resource_manager);
}