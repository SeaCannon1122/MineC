#include "minec_client.h"

#include "assets.h"

struct _asset
{
	void* data;
	size_t size;
	bool internal;
};

uint32_t asset_loader_create(struct minec_client* client)
{
	ASSET_LOADER.alloc = s_allocator_new(65536);
	ASSET_LOADER.resourcepack_paths_arraylist = arraylist_string_new_copy(SETTINGS.active_resourcepack_paths_arraylist);
	mutex_create(&ASSET_LOADER.mutex);

	return MINEC_CLIENT_SUCCESS;
}

void asset_loader_destroy(struct minec_client* client)
{
	mutex_destroy(&ASSET_LOADER.mutex);
	arraylist_string_delete(ASSET_LOADER.resourcepack_paths_arraylist);
	s_allocator_delete(ASSET_LOADER.alloc);
}

uint32_t asset_loader_sync_with_settings(struct minec_client* client)
{
	mutex_lock(&ASSET_LOADER.mutex);

	arraylist_string_delete(ASSET_LOADER.resourcepack_paths_arraylist);
	ASSET_LOADER.resourcepack_paths_arraylist = arraylist_string_new_copy(SETTINGS.active_resourcepack_paths_arraylist);

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