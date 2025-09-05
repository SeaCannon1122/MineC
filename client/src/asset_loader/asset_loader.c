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
			index_file_data = file_load(index_file_path, &index_file_size);
			s_free(alloc, index_file_path);
		}; 

		if (index_file_data)
		{
			void* indexed_assets_arraylist = arraylist_string_new(128);
			arraylist_string_read_file_data(indexed_assets_arraylist, index_file_data, index_file_size);

			for (uint32_t j = 0; j < arraylist_string_get_length(indexed_assets_arraylist); j++)
			{
				uint8_t* rel_asset_path = arraylist_string_get_element(indexed_assets_arraylist, j);

				if (i == -1)
				{
					uint32_t value = 0;
					hashmap_set_value(asset_paths_hashmap, rel_asset_path, &value, HASHMAP_VALUE_INT);
				}
				else
				{
					uint8_t* abs_asset_path = s_alloc_string(alloc, "%sassets/%s", resource_pack_path, rel_asset_path);
					hashmap_set_value(asset_paths_hashmap, rel_asset_path, abs_asset_path, HASHMAP_VALUE_STRING);
					s_free(alloc, abs_asset_path);
				}
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
	ASSET_LOADER.assets = calloc(ASSET_LOADER.asset_count, sizeof(struct asset_loader_asset));

	hashmap_iterator_start(&it, asset_paths_hashmap);
	for (uint32_t i = 0; value = hashmap_iterator_next_key_value_pair(&it, &key); i++)
	{
		if (value->type == HASHMAP_VALUE_STRING)
			ASSET_LOADER.assets[i].data = file_load(value, &ASSET_LOADER.assets[i].size);
		else
			ASSET_LOADER.assets[i].data = cerialized_get_file(cerialized_assets_file_system, key, &ASSET_LOADER.assets[i].size);

		if (ASSET_LOADER.assets[i].data)
		{
			hashmap_set_value(ASSET_LOADER.asset_names_hashmap, key, &i, HASHMAP_VALUE_INT);
			if (value->type == HASHMAP_VALUE_STRING) ASSET_LOADER.assets[i].is_external = true;
		}
	}

	hashmap_delete(asset_paths_hashmap);

	return MINEC_CLIENT_SUCCESS;
}

void _asset_loader_unload(struct minec_client* client)
{
	for (uint32_t i = 0; i < ASSET_LOADER.asset_count; i++) if (ASSET_LOADER.assets[i].is_external) free(ASSET_LOADER.assets[i].data);
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
		data = ASSET_LOADER.assets[value->data._int].data;
		*size = ASSET_LOADER.assets[value->data._int].size;
		atomic_uint32_t_store(&ASSET_LOADER.borrowed_asset_count, atomic_uint32_t_load(&ASSET_LOADER.borrowed_asset_count) + 1);
	}

	mutex_unlock(&ASSET_LOADER.mutex);
	return data;
}

void asset_loader_release_asset(struct minec_client* client)
{
	atomic_uint32_t_store(&ASSET_LOADER.borrowed_asset_count, atomic_uint32_t_load(&ASSET_LOADER.borrowed_asset_count) - 1);
}