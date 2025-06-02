#include "backend_opengl.h"

uint32_t renderer_backend_opengl_render(struct minec_client* client)
{
	uint32_t return_result = MINEC_CLIENT_SUCCESS;
	uint32_t result = MINEC_CLIENT_SUCCESS;

	struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;
	struct renderer_backend_opengl_pipelines_resources* pipelines_resources = client->renderer.backend.pipelines_resources.pipelines_resources;

	if (window_glMakeCurrent(client->window.window_handle) == false)
	{
		renderer_backend_opengl_log(client, "window_glMakeCurrent failed");
		return MINEC_CLIENT_ERROR;
	}

	uint32_t width;
	uint32_t height;
	atomic_load_(uint32_t, &client->window.width, &width);
	atomic_load_(uint32_t, &client->window.height, &height);

	uint32_t fps_new;
	atomic_load_(uint32_t, &base->fps_new, &fps_new);

	if (fps_new != base->fps)
	{
		/*fps change*/
		base->fps = fps_new;
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		base->func.glViewport(0, 0, width, height);
		base->func.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		base->func.glClear(GL_COLOR_BUFFER_BIT);
		result = gl_error_check_log(client, base, "Initializing frame");
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (pipelines_resources->pixelchar_renderer.usable == true)
			pixelcharRendererBackendOpenGLRender(client->renderer.pixelchar_renderer, width, height, 4.f, 4.f, 4.f, 1.4f);
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glSwapBuffers(client->window.window_handle) == false) 
		{
			renderer_backend_opengl_log(client, "window_glSwapBuffers failed");
			return_result = MINEC_CLIENT_ERROR;
			result = MINEC_CLIENT_ERROR;
		}
	}

	window_glMakeCurrent(NULL);
	return return_result;
}