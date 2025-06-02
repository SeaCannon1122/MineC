#include "backend_opengl.h"

uint32_t renderer_backend_opengl_device_create(struct minec_client* client, void** backend_base, void** backend_device, uint32_t device_index, uint32_t fps)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		renderer_backend_opengl_log(client, "window_glMakeCurrent failed");
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_opengl_base* base = *backend_base;
	uint32_t result = MINEC_CLIENT_SUCCESS;

	if (window_glSwapInterval(fps == 0 ? 1 : 0) == false)
	{
		renderer_backend_opengl_log(client, "window_glSwapInterval failed");
		result = MINEC_CLIENT_ERROR;
	}
	else base->fps = fps;

	window_glMakeCurrent(NULL);
	return result;
}

void renderer_backend_opengl_device_destroy(struct minec_client* client, void** backend_base, void** backend_device)
{

}

uint32_t renderer_backend_opengl_set_fps(struct minec_client* client, uint32_t fps)
{
	struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;

	atomic_store_(uint32_t, &base->fps_new, &fps);
	return MINEC_CLIENT_SUCCESS;
}