#include "minec_client.h"

#include "assets.h"

uint32_t _asset_loader_load(struct minec_client* client)
{
	void* asset_paths_hashmap = hashmap_new(1024, 4);
	void* alloc = s_allocator_new(2048);

	for (int32_t i = -1; i < (int32_t)arraylist_string_get_length(SETTINGS.active_resourcepack_paths_arraylist); i++)
	{
		uint8_t* resource_pack_path;
		
		size_t index_file_size;
		void* index_file_data;

		if (i == -1) index_file_data = cerialized_get_file(cerialized_assets_file_system, "index.txt", &index_file_size);
		else
		{
			resource_pack_path = arraylist_string_get_element(SETTINGS.active_resourcepack_paths_arraylist, i);

			uint8_t* index_file_path = s_alloc_string(alloc, "%sassets/index.txt", resource_pack_path);
			index_file_data = file_load(index_file_path, index_file_data);
			s_free(alloc, index_file_path);
		}; 

		if (index_file_data)
		{
			void* indexed_assets_arraylist = arraylist_string_new(128);
			arraylist_string_read_file_data(indexed_assets_arraylist, index_file_data, index_file_size);

			for (uint32_t j = 0; j < arraylist_string_get_length(indexed_assets_arraylist); j++)
			{
				uint8_t* rel_asset_path = arraylist_string_get_element(indexed_assets_arraylist, j);
				uint8_t* abs_asset_path;

				if (i == -1) abs_asset_path = s_alloc_string(alloc, ":%s", rel_asset_path);
				else abs_asset_path = s_alloc_string(alloc, "%sassets/%s", resource_pack_path, rel_asset_path);

				hashmap_set_value(asset_paths_hashmap, rel_asset_path, abs_asset_path, HASHMAP_VALUE_STRING);

				s_free(alloc, abs_asset_path);
			}

			arraylist_string_delete(indexed_assets_arraylist);
			if (i != -1) free(index_file_data);
		}
	}
	s_allocator_delete(alloc);

	ASSET_LOADER.asset_names_hashmap = hashmap_new(1024, 4);
	ASSET_LOADER.asset_count = 0;

	struct hashmap_iterator it;
	uint8_t* key;
	struct hashmap_multi_type* value;

	hashmap_iterator_start(&it, asset_paths_hashmap);
	while (value = hashmap_iterator_next_key_value_pair(&it, &key)) ASSET_LOADER.asset_count++;

	hashmap_iterator_start(&it, asset_paths_hashmap);
	while (value = hashmap_iterator_next_key_value_pair(&it, &key))

	hashmap_delete(asset_paths_hashmap);
}

void _asset_loader_unload(struct minec_client* client)
{
	for (uint32_t i = 0; i < ASSET_LOADER.asset_count; i++) if (ASSET_LOADER.assets[i].is_external) free(ASSET_LOADER.assets[i].data);
	free(ASSET_LOADER.assets);
	hashmap_delete(ASSET_LOADER.asset_names_hashmap);
}

uint32_t asset_loader_create(struct minec_client* client)
{
	ASSET_LOADER.alloc = s_allocator_new(65536);
	mutex_create(&ASSET_LOADER.mutex);

	return MINEC_CLIENT_SUCCESS;
}

void asset_loader_destroy(struct minec_client* client)
{
	mutex_destroy(&ASSET_LOADER.mutex);
	s_allocator_delete(ASSET_LOADER.alloc);
}

uint32_t asset_loader_reload(struct minec_client* client)
{
	_asset_loader_unload(client);
	_asset_loader_load(client);
}

void* asset_loader_get_asset(struct minec_client* client, uint8_t* name, size_t* size)
{

}

uint32_t asset_loader_sync_with_settings(struct minec_client* client)
{
	mutex_lock(&ASSET_LOADER.mutex);

	arraylist_string_delete(ASSET_LOADER.resourcepack_paths_arraylist);
	ASSET_LOADER.

	mutex_unlock(&ASSET_LOADER.mutex);
	return MINEC_CLIENT_SUCCESS;
}

void* asset_loader_asset_load(struct minec_client* client, uint8_t* name, uint8_t** data, size_t* size)
{
	mutex_lock(&ASSET_LOADER.mutex);

	struct _asset* asset = s_alloc(ASSET_LOADER.alloc, sizeof(struct _asset));
	asset->data = NULL;
	asset->internal = true;

	for (uint32_t i = 0; i < arraylist_string_get_length(ASSET_LOADER.resourcepack_paths_arraylist); i++)
	{
		uint8_t* path_components[3] = { arraylist_string_get_element(ASSET_LOADER.resourcepack_paths_arraylist, i), "assets/", name};
		uint8_t* path = s_alloc_joined_string(ASSET_LOADER.alloc, path_components, 3);
		asset->data = file_load(path, &asset->size);
		s_free(ASSET_LOADER.alloc, path);
		
		if (asset->data != NULL)
		{
			asset->internal = false;
			break;
		}
	}

	if (asset->data == NULL) asset->data = cerialized_get_file(cerialized_assets_file_system, name, &asset->size);

	if (asset->data == NULL)
	{
		s_free(ASSET_LOADER.alloc, asset);
		asset = NULL;
	}
	else
	{
		*data = asset->data;
		*size = asset->size;
	}

	mutex_unlock(&ASSET_LOADER.mutex);

	return asset;
}

void asset_loader_asset_unload(struct minec_client* client, void* asset_handle)
{
	mutex_lock(&ASSET_LOADER.mutex);

	struct _asset* asset = asset_handle;
	if (asset->internal == false) free(asset->data);
	s_free(ASSET_LOADER.alloc, asset);

	mutex_unlock(&ASSET_LOADER.mutex);
}