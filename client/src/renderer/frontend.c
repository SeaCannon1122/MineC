#include "renderer_internal.h"

uint32_t renderer_frontend_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		pixelchar_renderer_created = false
	;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (pixelcharRendererCreate(1024, &RENDERER.frontend.pixelchar_renderer) != PIXELCHAR_SUCCESS)
		{
			minec_client_log_debug_l(client, "'pixelcharRendererCreate' failed");
			result = MINEC_CLIENT_SUCCESS;
		}
		else pixelchar_renderer_created = true;
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{

	}

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (pixelchar_renderer_created) pixelcharRendererDestroy(RENDERER.frontend.pixelchar_renderer);
	}

	return result;
}

void renderer_frontend_destroy(struct minec_client* client)
{
	pixelcharRendererDestroy(RENDERER.frontend.pixelchar_renderer);
}

void renderer_frontend_frame(struct minec_client* client)
{

}

void renderer_frontend_reload_resources(struct minec_client* client)
{

}

void renderer_frontend_set_gui_scale(struct minec_client* client, uint32_t scale)
{

}

void renderer_frontend_set_fov(struct minec_client* client, uint32_t fov)
{

}

void renderer_frontend_set_render_distance(struct minec_client* client, uint32_t render_distance)
{

}