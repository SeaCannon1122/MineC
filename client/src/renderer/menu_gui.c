#include <minec_client.h>

static void _load_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < MENU_TEXTURE_s_COUNT; i++)
	{
		size_t raw_data_size;
		uint8_t* raw_data = asset_loader_get_asset(client, menu_texture_names[i], &raw_data_size);

		if (raw_data)
		{
			RENDERER.components.menu.tetxures[i].data = stbi_load_from_memory(
				raw_data,
				raw_data_size,
				&RENDERER.components.menu.tetxures[i].width,
				&RENDERER.components.menu.tetxures[i].width,
				&RENDERER.components.menu.tetxures[i].original_channels,
				4
			);
		}
		else RENDERER.components.menu.tetxures[i].data = NULL;
	}
}

static void _unload_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < MENU_TEXTURE_s_COUNT; i++) if (RENDERER.components.menu.tetxures[i].data) free(RENDERER.components.menu.tetxures[i].data);
}

uint32_t renderer_component_menu_gui_create(struct minec_client* client)
{
	_load_assets(client);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_component_menu_gui_destroy(struct minec_client* client)
{
	_unload_assets(client);
}

uint32_t renderer_backend_component_menu_gui_create(struct minec_client* client)
{
	_load_assets(client);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_component_menu_gui_destroy(struct minec_client* client)
{
	_unload_assets(client);
}

uint32_t renderer_component_menu_gui_reload_assets(struct minec_client* client)
{
	_unload_assets(client);
	_load_assets(client);
}

uint32_t renderer_component_menu_gui_frame(struct minec_client* client)
{

}
