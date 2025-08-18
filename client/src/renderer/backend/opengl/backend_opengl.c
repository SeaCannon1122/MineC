#include <minec_client.h>

bool _opengl_error_get_log(struct minec_client* client, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line)
{
	GLenum error = OPENGL.func.glGetError();
	if (error != GL_NO_ERROR)
	{
		uint8_t* error_name = "Unknown";
		switch (error)
		{
		case GL_INVALID_ENUM: { error_name = "GL_INVALID_ENUM"; break; }
		case GL_INVALID_VALUE: { error_name = "GL_INVALID_VALUE"; break; }
		case GL_INVALID_OPERATION: { error_name = "GL_INVALID_OPERATION"; break; }
		case GL_OUT_OF_MEMORY: { error_name = "GL_OUT_OF_MEMORY"; break; }
		case GL_INVALID_FRAMEBUFFER_OPERATION: { error_name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; }
		}

		_minec_client_log_debug_l(client, function, file, line, "'%s' failed with glError 0x%x '%s'", action, error, error_name);
		minec_client_log_error(client, "[RENDERER] [OPENGL] Received OpenGL error 0x%x '%s'", error, error_name);
	}

	return (error != GL_NO_ERROR);
}

uint32_t opengl_errors_clear(struct minec_client* client)
{
	for (uint32_t i = 0; OPENGL.func.glGetError() != GL_NO_ERROR; i++)
		if (i > 64) return MINEC_CLIENT_ERROR;
	return MINEC_CLIENT_SUCCESS;
}

uint32_t _context_create(struct minec_client* client);
void _context_destroy(struct minec_client* client);
uint32_t _context_frame_submit(struct minec_client* client);

#ifdef MINEC_CLIENT_DEBUG_LOG
uint32_t _debug_messenger_init(struct minec_client* client);
#endif

void pixelchar_shader_error_log_function(struct minec_client* client, uint8_t* message)
{
	minec_client_log_error(client, "[RENDERER] [OPENGL] ", message);
}

uint32_t renderer_backend_opengl_create(struct minec_client* client)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		context_created = false,
		pixelchar_created = false
	;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((result = _context_create(client)) != MINEC_CLIENT_SUCCESS) minec_client_log_error(client, "[RENDERER] [OPENGL] Failed to create usable OpenGL context");
		else context_created = true;
	}
	
#ifdef MINEC_CLIENT_DEBUG_LOG
	if (result == MINEC_CLIENT_ERROR) result = _debug_messenger_init(client);
#endif

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (pixelcharRendererBackendOpenGLInitialize(
			RENDERER.components.pixelchar.renderer,
			RENDERER.backend.pixelchar_slot + RENDERER.backend.pixelchar_slot_offset,
			OPENGL_RESOURCE_FRAME_COUNT,
			window_glGetProcAddress,
			NULL, 0, NULL, 0,
			pixelchar_shader_error_log_function,
			client
		) != PIXELCHAR_SUCCESS)
		{
			result = MINEC_CLIENT_ERROR;
		}
		else pixelchar_created = true;
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		OPENGL.func.glEnable(GL_BLEND);
		OPENGL.func.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		OPENGL.func.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (pixelchar_created) pixelcharRendererBackendOpenGLDeinitialize(RENDERER.components.pixelchar.renderer, RENDERER.backend.pixelchar_slot + RENDERER.backend.pixelchar_slot_offset);
		if (context_created) _context_destroy(client);
	}

	return result;
}

void renderer_backend_opengl_destroy(struct minec_client* client)
{
	pixelcharRendererBackendOpenGLDeinitialize(RENDERER.components.pixelchar.renderer, RENDERER.backend.pixelchar_slot + RENDERER.backend.pixelchar_slot_offset);
	_context_destroy(client);
}

uint32_t renderer_backend_opengl_reload_assets(struct minec_client* client)
{

}

uint32_t renderer_backend_opengl_switch_device(struct minec_client* client, uint32_t device_index)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_reload_resources(struct minec_client* client)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_set_vsync(struct minec_client* client, bool vsync)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_set_fps(struct minec_client* client, uint32_t fps)
{
	RENDERER.backend.settings.fps = fps;

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_frame_begin(struct minec_client* client)
{
	OPENGL.func.glViewport(0, 0, RENDERER.frame_info.width, RENDERER.frame_info.height);
	OPENGL.func.glClear(GL_COLOR_BUFFER_BIT);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_frame_menu(struct minec_client* client)
{
	pixelcharRendererBackendOpenGLRender(
		RENDERER.components.pixelchar.renderer,
		RENDERER.backend.pixelchar_slot + RENDERER.backend.pixelchar_slot_offset,
		OPENGL.resource_frame_index,
		RENDERER.frame_info.width,
		RENDERER.frame_info.height,
		4.f, 4.f, 4.f, 1.4f
	);

	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_frame_end(struct minec_client* client)
{
	_context_frame_submit(client);

	return MINEC_CLIENT_SUCCESS;
}