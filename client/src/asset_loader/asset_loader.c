#include "minec_client.h"

uint32_t asset_loader_create(struct minec_client* client)
{
	if ((ASSET_LOADER.ressourcepack_path_arraylist = arraylist_string_new_copy(SETTINGS.active_resourcepack_paths_arraylist)) == NULL) return MINEC_CLIENT_ERROR;
	
	uint32_t ressourcepack_path_arraylist_length = arraylist_string_get_length(ASSET_LOADER.ressourcepack_path_arraylist);
	if (ressourcepack_path_arraylist_length > ASSET_LOADER_MAX_RESOURCE_PACKS) arraylist_string_remove_elements(ASSET_LOADER.ressourcepack_path_arraylist, ASSET_LOADER_MAX_RESOURCE_PACKS, ressourcepack_path_arraylist_length - 1);

	uint32_t length = arraylist_string_get_length(ASSET_LOADER.ressourcepack_path_arraylist);

	for (uint32_t i = 0; i <= length; i++)
	{
		if ((ASSET_LOADER.mapping_hashmaps[i] = hashmap_new(128, 4)) == NULL)
		{
			for (uint32_t j = 0; j < i; j++) hashmap_delete(ASSET_LOADER.mapping_hashmaps[j]);
			arraylist_string_delete(ASSET_LOADER.ressourcepack_path_arraylist);
			return MINEC_CLIENT_ERROR;
		}

		uint8_t* mappings_file_path = string8_malloc("%sassets/mappings.yaml", i < length ? arraylist_string_get_element(ASSET_LOADER.ressourcepack_path_arraylist, i) : "");

		size_t* mappings_file_size;
		void* mappings_file_data;

		if (i < length) mappings_file_data = file_load(mappings_file_path, &mappings_file_size);
		else mappings_file_data = resource_index_query("assets/mappings.yaml", &mappings_file_size);

		if (mappings_file_data)
		{
			hashmap_read_yaml(ASSET_LOADER.mapping_hashmaps[i], mappings_file_data, mappings_file_size);
			if (i < length) free(mappings_file_data);
		}

		free(mappings_file_path);
	}

	return MINEC_CLIENT_SUCCESS;
}

void asset_loader_destroy(struct minec_client* client)
{
	for (uint32_t i = 0; i <= arraylist_string_get_length(ASSET_LOADER.ressourcepack_path_arraylist); i++) hashmap_delete(ASSET_LOADER.mapping_hashmaps[i]);
	arraylist_string_delete(ASSET_LOADER.ressourcepack_path_arraylist);
}


bool asset_loader_get_asset(struct minec_client* client, uint8_t* name, struct asset_loader_asset* asset)
{
	asset->data = NULL;
	asset->external = false;
	uint32_t length = arraylist_string_get_length(ASSET_LOADER.ressourcepack_path_arraylist);

	for (uint32_t i = 0; i <= length && asset->data == NULL; i++)
	{
		uint8_t* asset_name = name;

		const struct hashmap_multi_type* hashmap_value;
		if (hashmap_value = hashmap_get_value(ASSET_LOADER.mapping_hashmaps[i], name)) 
			if (hashmap_value->type == HASHMAP_VALUE_STRING8) 
				asset_name = hashmap_value->data_string8;

		uint8_t* path = string8_malloc("%sassets/%s", i < length ? arraylist_string_get_element(ASSET_LOADER.ressourcepack_path_arraylist, i) : "", asset_name);

		if (i < length) { if (asset->data = file_load(path, &asset->size)) asset->external = true; }
		else asset->data = resource_index_query(path, &asset->size);

		free(path);
	}

	if (asset->data) return true;
	else return false;
}