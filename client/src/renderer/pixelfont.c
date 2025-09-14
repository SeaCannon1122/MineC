#include <minec_client.h>

static void load_font(struct minec_client* client)
{
	size_t raw_data_size;
	uint8_t* raw_data = asset_loader_get_asset(client, "minec/font.pixelfont", &raw_data_size);

	if (raw_data)
	{

	}
	else
	{
	}
}

uint32_t renderer_component_pixelfont_create(struct minec_client* client)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_component_pixelfont_destroy(struct minec_client* client)
{

}

uint32_t renderer_backend_component_pixelfont_create(struct minec_client* client)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_component_pixelfont_destroy(struct minec_client* client)
{

}

uint32_t renderer_component_pixelfont_frame(struct minec_client* client)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_component_pixelfont_reload_assets(struct minec_client* client)
{
	return MINEC_CLIENT_SUCCESS;
}