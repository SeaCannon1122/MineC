#include "renderer_internal.h"

uint32_t renderer_frontend_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	RENDERER.frontend.settings = RENDERER.public.request.settings.frontend;

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

	RENDERER.public.request.settings.frontend = RENDERER.frontend.settings;
	RENDERER.public.info.state.settings.frontend = RENDERER.frontend.settings;

	return result;
}

void renderer_frontend_destroy(struct minec_client* client)
{
	pixelcharRendererDestroy(RENDERER.frontend.pixelchar_renderer);
}

void renderer_frontend_frame(struct minec_client* client)
{
	ACCESS_INFO_STATE(ACCESS_REQUEST_STATE(
		if (
			RENDERER.public.request.settings.frontend.fov != RENDERER.frontend.settings.fov &&
			RENDERER.public.request.settings.frontend.fov > 0 &&
			RENDERER.public.request.settings.frontend.fov < 200
			)
		{
			RENDERER.frontend.settings.fov = RENDERER.public.request.settings.frontend.fov;
			RENDERER.public.info.state.settings.frontend.fov = RENDERER.frontend.settings.fov;
			RENDERER.public.info.changed = true;
		}
		else RENDERER.public.request.settings.frontend.fov == RENDERER.frontend.settings.fov;

		if (
			RENDERER.public.request.settings.frontend.gui_scale != RENDERER.frontend.settings.gui_scale &&
			RENDERER.public.request.settings.frontend.gui_scale <= 255
			)
		{
			RENDERER.frontend.settings.gui_scale = RENDERER.public.request.settings.frontend.gui_scale;
			RENDERER.public.info.state.settings.frontend.gui_scale = RENDERER.frontend.settings.gui_scale;
			RENDERER.public.info.changed = true;
		}
		else RENDERER.public.request.settings.frontend.gui_scale == RENDERER.frontend.settings.gui_scale;

		if (
			RENDERER.public.request.settings.frontend.render_distance != RENDERER.frontend.settings.render_distance &&
			RENDERER.public.request.settings.frontend.render_distance <= 255
			)
		{
			RENDERER.frontend.settings.render_distance = RENDERER.public.request.settings.frontend.render_distance;
			RENDERER.public.info.state.settings.frontend.render_distance = RENDERER.frontend.settings.render_distance;
			RENDERER.public.info.changed = true;
		}
		else RENDERER.public.request.settings.frontend.render_distance == RENDERER.frontend.settings.render_distance;
	));

	application_window_get_dimensions(client, &RENDERER.frontend.frame_info.width, &RENDERER.frontend.frame_info.height);
}