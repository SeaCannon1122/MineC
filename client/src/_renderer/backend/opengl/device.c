#include "backend_opengl.h"

uint32_t renderer_backend_opengl_device_create(struct minec_client* client, void** backend_base, void** backend_device, uint32_t device_index, uint32_t fps, bool vsync)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		minec_client_log_debug_l(client, "'window_glMakeCurrent' failed");
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_opengl_base* base = *backend_base;
	uint32_t result = MINEC_CLIENT_SUCCESS;


	if (result == MINEC_CLIENT_SUCCESS)
	{
		minec_client_retrieve_log_opengl_errors(client, base, &result, "Initial error reset");
		result = MINEC_CLIENT_SUCCESS;
	} minec_client_retrieve_log_opengl_errors(client, base, &result, "Initial error reset");

	uint32_t width, height;
	atomic_load_(uint32_t, &client->window.width, &width);
	atomic_load_(uint32_t, &client->window.height, &height);

	create_framebuffers(client, base);
	create_framebuffer_attachments(client, base, width, height);

	uint32_t swap_interval = (vsync ? 1 : 0);

	if (window_glSwapIntervalEXT(swap_interval) == false && swap_interval == 1)
	{
		minec_client_log_debug_l(client, "window_glSwapInterval(%d) failed", swap_interval);
		result = MINEC_CLIENT_ERROR;
	}
	else
	{
		base->fps = fps;
		base->last_frame_time = 0.f;
	}
	
	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
		minec_client_log_debug_l(client, "'window_glMakeCurrent(NULL)' failed");
		minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
	}

	return result;
}

void renderer_backend_opengl_device_destroy(struct minec_client* client, void** backend_base, void** backend_device)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		minec_client_log_debug_l(client, "'window_glMakeCurrent' failed");
		return;
	}

	struct renderer_backend_opengl_base* base = *backend_base;

	destroy_framebuffer_attachments(client, base);
	destroy_framebuffers(client, base);

	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
		minec_client_log_debug_l(client, "'window_glMakeCurrent(NULL)' failed");
		minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
	}
}

uint32_t renderer_backend_opengl_set_fps(struct minec_client* client, uint32_t fps)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		minec_client_log_debug_l(client, "'window_glMakeCurrent' failed");
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_backend_opengl_base* base = client->RENDERER.backend.base.base;
	uint32_t result = MINEC_CLIENT_SUCCESS;

	uint32_t swap_interval = (fps == 0 ? 1 : 0);

	if (window_glSwapInterval(swap_interval) == false)
	{
		minec_client_log_debug_l(client, "window_glSwapInterval(%d) failed", swap_interval);
		result = MINEC_CLIENT_ERROR;
	}
	else
	{
		base->fps = fps;
		base->last_frame_time = 0.f;
	}

	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
		minec_client_log_debug_l(client, "'window_glMakeCurrent(NULL)' failed");
		minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
	}

	return result;
}