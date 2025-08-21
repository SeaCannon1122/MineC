#include <minec_client.h>

static void _load_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < MENU_TEXTURE_s_COUNT; i++)
	{
		uint8_t* raw_data;
		size_t raw_data_size;
		void* asset = asset_loader_asset_load(client, menu_texture_names[i], &raw_data, &raw_data_size);

		if (asset)
		{
			RENDERER.components.menu.tetxures[i].data = stbi_load_from_memory(
				raw_data,
				raw_data_size,
				&RENDERER.components.menu.tetxures[i].width,
				&RENDERER.components.menu.tetxures[i].width,
				&RENDERER.components.menu.tetxures[i].original_channels,
				4
			);

			asset_loader_asset_unload(client, asset);
		}
		else RENDERER.components.menu.tetxures[i].data = NULL;
	}
}

static void _unload_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < MENU_TEXTURE_s_COUNT; i++) if (RENDERER.components.menu.tetxures[i].data) free(RENDERER.components.menu.tetxures[i].data);
}

uint32_t renderer_component_menu_create(struct minec_client* client)
{
	_load_assets(client);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_component_menu_destroy(struct minec_client* client)
{
	_unload_assets(client);
}

void renderer_component_menu_reload_assets(struct minec_client* client)
{
	_unload_assets(client);
	_load_assets(client);
}

void renderer_component_menu_frame(struct minec_client* client)
{
	uint8_t buffer[] = "Hello World!";

	Pixelchar c[sizeof(buffer) - 1];
	uint32_t scale = 2;

	for (uint32_t i = 0; i < sizeof(buffer) - 1; i++)
	{
		c[i].character = buffer[i];
		c[i].flags = PIXELCHAR_BACKGROUND_BIT | PIXELCHAR_UNDERLINE_BIT | PIXELCHAR_SHADOW_BIT;
		c[i].fontIndex = i % 2;
		c[i].scale = scale;

		c[i].position[1] = 100;

		if (i == 0) c[i].position[0] = 100;
		else c[i].position[0] = c[i - 1].position[0] + pixelcharGetCharacterRenderingWidth(RENDERER.components.pixelchar.renderer, &c[i - 1]) + pixelcharGetCharacterRenderingSpacing(RENDERER.components.pixelchar.renderer, &c[i - 1], &c[i]);

		c[i].color[0] = 0xdc;
		c[i].color[1] = 0xdc;
		c[i].color[2] = 0xdc;
		c[i].color[3] = 255;
		c[i].backgroundColor[0] = 255;
		c[i].backgroundColor[1] = 0;
		c[i].backgroundColor[2] = 0;
		c[i].backgroundColor[3] = 255;

	}

	PixelcharManagerResetQueue(RENDERER.components.pixelchar.renderer);
	PixelcharManagerEnqueCharacters(RENDERER.components.pixelchar.renderer, c, sizeof(buffer) - 1);

	renderer_backend_frame_menu(client);
}