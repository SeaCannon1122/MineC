#include <minec_client.h>

static void _load_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		RENDERER.components.pixelchar.fonts[i] = NULL;

		uint8_t name_buffer[64];
		snprintf(name_buffer, sizeof(name_buffer), "minec/fonts/font%u.pixelfont", i);

		size_t size;
		uint8_t* data;
		void* asset = asset_loader_asset_load(client, name_buffer, &data, &size);

		if (asset)
		{
			PixelcharResult presult;
			if ((presult = pixelcharFontCreate(data, size, &RENDERER.components.pixelchar.fonts[i])) == PIXELCHAR_SUCCESS)
			{
				PixelcharManagerBindFont(RENDERER.components.pixelchar.renderer, RENDERER.components.pixelchar.fonts[i], i);
				pixelcharFontGetName(RENDERER.components.pixelchar.fonts[i], RENDERER.components.pixelchar.font_names[i]);
			}
			else minec_client_log_debug_l(client, "'pixelcharFontCreate' failed with PixelcharError '%s'", pixelcharGetResultAsString(presult));

			asset_loader_asset_unload(client, asset);
		}
	}
}

static void _unload_assets(struct minec_client* client)
{
	for (uint32_t i = 0; i < PIXELCHAR_RENDERER_MAX_FONT_COUNT; i++)
	{
		if (RENDERER.components.pixelchar.fonts[i] != NULL)
		{
			PixelcharManagerBindFont(RENDERER.components.pixelchar.renderer, NULL, i);
			pixelcharFontDestroy(RENDERER.components.pixelchar.fonts[i]);
		}
	}
}

uint32_t renderer_component_pixelchar_create(struct minec_client* client)
{
	PixelcharResult presult;

	if ((presult = PixelcharManagerCreate(RENDERER_PIXELCHAR_RENDERER_QUEUE_LENGTH, &RENDERER.components.pixelchar.renderer)) != PIXELCHAR_SUCCESS)
	{
		minec_client_log_debug_l(client, "'PixelcharManagerCreate' failed with PixelcharError '%s'", pixelcharGetResultAsString(presult));
		return MINEC_CLIENT_ERROR;
	}

	_load_assets(client);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_component_pixelchar_destroy(struct minec_client* client)
{
	_unload_assets(client);
	PixelcharManagerDestroy(RENDERER.components.pixelchar.renderer);
}

void renderer_component_pixelchar_reload_assets(struct minec_client* client)
{
	_unload_assets(client);
	_load_assets(client);

	return MINEC_CLIENT_SUCCESS;
}