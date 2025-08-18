#include <minec_client.h>

uint32_t renderer_texture_manager_create(struct minec_client* client)
{
	if ((RENDERER.frontend.texture_manager.texture_index_hashmap = hashmap_new(2048, 4)) == NULL)
	{
		minec_client_log_debug_l(client, "'hashmap_new(2048, 4)' failed");
		return MINEC_CLIENT_ERROR;
	}

	if ((RENDERER.frontend.texture_manager.tetxures = malloc(2048 * sizeof(struct frontend_texture))) == NULL)
	{
		minec_client_log_out_of_memory(client, "[RENDERER] [FRONTEND]", "malloc");
		hashmap_delete(RENDERER.frontend.texture_manager.texture_index_hashmap);
		return MINEC_CLIENT_ERROR;
	}

	RENDERER.frontend.texture_manager.textures_count = 0;
	RENDERER.frontend.texture_manager.textures_allocated_count = 2048;

	return MINEC_CLIENT_SUCCESS;
}

void texture_manager_add_texture(struct minec_client* client, uint8_t* name)
{
	if (hashmap_get_value(RENDERER.frontend.texture_manager.texture_index_hashmap, name) != NULL) return;

	uint8_t* raw_data;
	size_t raw_data_size;
	
	void* asset = asset_loader_asset_load(client, name, &raw_data, &raw_data_size);
	if (asset != NULL)
	{
		struct frontend_texture texture;
		if ((texture.data = stbi_load_from_memory(raw_data, raw_data_size, &texture.width, &texture.height, &texture.original_channels, 4)) != NULL)
		{
			if (RENDERER.frontend.texture_manager.textures_count == RENDERER.frontend.texture_manager.textures_allocated_count)
			{
				RENDERER.frontend.texture_manager.textures_allocated_count += 256;
				RENDERER.frontend.texture_manager.tetxures = realloc(RENDERER.frontend.texture_manager.tetxures, RENDERER.frontend.texture_manager.textures_allocated_count * sizeof(struct frontend_texture));
			}

			RENDERER.frontend.texture_manager.tetxures[RENDERER.frontend.texture_manager.textures_count] = texture;
			hashmap_set_value(RENDERER.frontend.texture_manager.texture_index_hashmap, name, &RENDERER.frontend.texture_manager.textures_count, HASHMAP_VALUE_INT);
			RENDERER.frontend.texture_manager.textures_count++;

			free(texture.data);
		}
		asset_loader_asset_unload(client, asset);
	}
}

void renderer_texture_manager_free(struct minec_client* client)
{
	for (uint32_t i = 0; i < RENDERER.frontend.texture_manager.textures_count; i++) free(RENDERER.frontend.texture_manager.tetxures[i].data);
	RENDERER.frontend.texture_manager.textures_count = 0;
}

void renderer_texture_manager_destroy(struct minec_client* client)
{
	renderer_texture_manager_destroy_free(client);

	free(RENDERER.frontend.texture_manager.tetxures);
	hashmap_delete(RENDERER.frontend.texture_manager.texture_index_hashmap);
}