#include "backend_opengl.h"

uint32_t renderer_backend_opengl_start_rendering(struct minec_client* client)
{
	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		minec_client_log_debug_l(client, "'window_glMakeCurrent' failed");
		return MINEC_CLIENT_ERROR;
	}
	return MINEC_CLIENT_SUCCESS;
}

void  renderer_backend_opengl_stop_rendering(struct minec_client* client)
{
	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
		minec_client_log_debug_l(client, "'window_glMakeCurrent(NULL)' failed");
		minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
	}
}

uint32_t renderer_backend_opengl_render(struct minec_client* client, bool resize)
{
	uint32_t return_result = MINEC_CLIENT_SUCCESS;
	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_opengl_base* base = client->RENDERER.backend.base.base;
	struct renderer_backend_opengl_pipelines_resources* pipelines_resources = client->RENDERER.backend.pipelines_resources.pipelines_resources;
	 
	float frame_time = get_time();
	client->renderer.thread_state.frame_info.time = frame_time - base->last_frame_time;
	if (1000.f / (float)base->fps - client->renderer.thread_state.frame_info.time > 0 && base->fps != 0) sleep_for_ms((uint32_t)(1000.f / (float)base->fps - client->renderer.thread_state.frame_info.time));
	base->last_frame_time = get_time();

	uint32_t width, height;
	atomic_load_(uint32_t, &client->window.width, &width);
	atomic_load_(uint32_t, &client->window.height, &height);

	if (resize)
	{
		destroy_framebuffer_attachments(client, base);
		create_framebuffer_attachments(client, base, client->renderer.thread_state.rendering_width, client->renderer.thread_state.rendering_height);
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		base->func.glViewport(0, 0, width, height);
		base->func.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		base->func.glClear(GL_COLOR_BUFFER_BIT);
	}
	minec_client_retrieve_log_opengl_errors(client, base, &result, "Initializing frame");

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (pipelines_resources->pixelchar_renderer.usable == true)
		{
			PixelcharManagerBackendOpenGLRender(client->renderer.pixelchar_renderer, pipelines_resources->pixelchar_RENDERER.backend_index, 0, width, height, 4.f, 4.f, 4.f, 1.4f);
		}
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glSwapBuffers(client->window.window_handle) == false) 
		{
			minec_client_log_debug_l(client, "'window_glSwapBuffers' failed");
			return_result = MINEC_CLIENT_ERROR;
			result = MINEC_CLIENT_ERROR;
		}
	}
	return return_result;
}