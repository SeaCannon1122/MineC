#include "minec_client.h"

uint32_t _asset_loader_load(struct minec_client* client)
{
	void* asset_hashmap = hashmap_new(1024, 4);
	void* alloc = s_allocator_new(2048);

	uint32_t resource_pack_count = arraylist_string_get_length(SETTINGS.active_resourcepack_paths_arraylist);

	for (uint32_t i = 0; i <= resource_pack_count; i++)
	{
		uint8_t* resource_pack_path;
		
		size_t index_file_size;
		void* index_file_data;

		if (i == resource_pack_count) index_file_data = resource_index_query("assets/index.txt", &index_file_size);
		else
		{
			resource_pack_path = arraylist_string_get_element(SETTINGS.active_resourcepack_paths_arraylist, i);

			uint8_t* index_file_path = s_alloc_string(alloc, "%sassets/index.txt", resource_pack_path);
			index_file_data = file_load(index_file_path, &index_file_size);
			s_free(alloc, index_file_path);
		}; 

		if (index_file_data)
		{
			void* indexed_assets_arraylist = arraylist_string_new(128);
			arraylist_string_read_file_data(indexed_assets_arraylist, index_file_data, index_file_size);

			for (uint32_t j = 0; j < arraylist_string_get_length(indexed_assets_arraylist); j++)
			{
				hashmap_set_value(
					asset_hashmap, 
					arraylist_string_get_element(indexed_assets_arraylist, j), 
					&(uint32_t){0}, 
					HASHMAP_VALUE_INT32
				);
			}

			arraylist_string_delete(indexed_assets_arraylist);
			if (i != resource_pack_count) free(index_file_data);
		}
	}

	ASSET_LOADER.asset_names_hashmap = hashmap_new(1024, 4);
	ASSET_LOADER.asset_count = hashmap_get_key_count(asset_hashmap);
	ASSET_LOADER.assets = calloc(ASSET_LOADER.asset_count, sizeof(struct asset_loader_asset));

	struct hashmap_iterator it;
	uint8_t* key;

	hashmap_iterator_start(&it, asset_hashmap);
	for (uint32_t i = 0; hashmap_iterator_next_key_value_pair(&it, &key); i++)
	{
		for (uint32_t j = 0; j < resource_pack_count && ASSET_LOADER.assets[i].data == NULL; j++)
		{
			const uint8_t* resource_pack_path = arraylist_string_get_element(SETTINGS.active_resourcepack_paths_arraylist, j);

			uint8_t* asset_path = s_alloc_string(alloc, "%sassets/%s", resource_pack_path, key);
			ASSET_LOADER.assets[i].data = file_load(asset_path, &ASSET_LOADER.assets[i].size);
			s_free(alloc, asset_path);
		}

		if (ASSET_LOADER.assets[i].data) ASSET_LOADER.assets[i].external = true;
		else
		{
			uint8_t* asset_path = s_alloc_string(alloc, "assets/%s", key);
			ASSET_LOADER.assets[i].data = resource_index_query(asset_path, &ASSET_LOADER.assets[i].size);
			s_free(alloc, asset_path);
		}

		if (ASSET_LOADER.assets[i].data) hashmap_set_value(ASSET_LOADER.asset_names_hashmap, key, &i, HASHMAP_VALUE_INT32);
	}

	s_allocator_delete(alloc);
	hashmap_delete(asset_hashmap);

	return MINEC_CLIENT_SUCCESS;
}

void _asset_loader_unload(struct minec_client* client)
{
	for (uint32_t i = 0; i < ASSET_LOADER.asset_count; i++) if (ASSET_LOADER.assets[i].external) free(ASSET_LOADER.assets[i].data);
	free(ASSET_LOADER.assets);
	hashmap_delete(ASSET_LOADER.asset_names_hashmap);
}

uint32_t asset_loader_create(struct minec_client* client)
{
	mutex_create(&ASSET_LOADER.mutex);
	atomic_uint32_t_init(&ASSET_LOADER.borrowed_asset_count, 0);
	_asset_loader_load(client);

	return MINEC_CLIENT_SUCCESS;
}

void asset_loader_destroy(struct minec_client* client)
{
	atomic_uint32_t_deinit(&ASSET_LOADER.borrowed_asset_count);
	_asset_loader_unload(client);
	mutex_destroy(&ASSET_LOADER.mutex);
}

void asset_loader_reload(struct minec_client* client)
{
	mutex_lock(&ASSET_LOADER.mutex);

	while (atomic_uint32_t_load(&ASSET_LOADER.borrowed_asset_count) != 0) time_sleep(1);

	_asset_loader_unload(client);
	_asset_loader_load(client);

	mutex_unlock(&ASSET_LOADER.mutex);
}

void* asset_loader_get_asset(struct minec_client* client, uint8_t* name, size_t* size)
{
	void* data = NULL;
	mutex_lock(&ASSET_LOADER.mutex);

	struct hashmap_multi_type* value;
	if (value = hashmap_get_value(ASSET_LOADER.asset_names_hashmap, name))
	{
		data = ASSET_LOADER.assets[value->data_int32].data;
		*size = ASSET_LOADER.assets[value->data_int32].size;
		atomic_uint32_t_store(&ASSET_LOADER.borrowed_asset_count, atomic_uint32_t_load(&ASSET_LOADER.borrowed_asset_count) + 1);
	}

	mutex_unlock(&ASSET_LOADER.mutex);
	return data;
}

void asset_loader_release_asset(struct minec_client* client)
{
	atomic_uint32_t_store(&ASSET_LOADER.borrowed_asset_count, atomic_uint32_t_load(&ASSET_LOADER.borrowed_asset_count) - 1);
}