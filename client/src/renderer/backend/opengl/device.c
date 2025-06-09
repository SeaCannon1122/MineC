#include "backend_opengl.h"

uint32_t renderer_backend_opengl_device_create(struct minec_client* client, void** backend_base, void** backend_device, uint32_t device_index, uint32_t fps)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		minec_client_log_debug_error(client, "'window_glMakeCurrent' failed");
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_opengl_base* base = *backend_base;
	uint32_t result = MINEC_CLIENT_SUCCESS;

	uint32_t swa_interval = (fps == 0 ? 1 : 0);

	if (window_glSwapInterval(swa_interval) == false)
	{
		minec_client_log_debug_error(client, "window_glSwapInterval(%d) failed", swa_interval);
		result = MINEC_CLIENT_ERROR;
	}
	else
	{
		base->fps = fps;
		atomic_store_(uint32_t, &base->fps_new, &fps);
		base->last_frame_time = 0.f;
	}
	
	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
		minec_client_log_debug_error(client, "'window_glMakeCurrent(NULL)' failed");
		minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
	}

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