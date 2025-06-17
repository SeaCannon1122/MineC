#include "renderer/renderer_internal.h"


uint32_t _opengl_create_context(struct minec_client* client, bool* vsync_disable_support)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		opengl_loaded = false,
		opengl_context_created = false
	;

	bool glSwapIntervalEXT_support;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_opengl_load() == true) opengl_loaded = true;
		else
		{
			minec_client_log_debug_error(client, "window_opengl_load() failed");
			result = MINEC_CLIENT_ERROR;
		}
		 
	}   

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glCreateContext(client->window.window_handle, 4, 3, NULL, &glSwapIntervalEXT_support) == true) opengl_context_created = true;
		else
		{
			minec_client_log_debug_error(client, "window_glCreateContext() with version 4.3 failed");
			result = MINEC_CLIENT_ERROR;
		}
	}

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (opengl_loaded) window_opengl_unload();
		if (opengl_context_created) window_glDestroyContext(client->window.window_handle);
	}
}

void _opengl_destroy_context(struct minec_client* client)
{

	window_glDestroyContext(client->window.window_handle);
	window_opengl_unload();
}