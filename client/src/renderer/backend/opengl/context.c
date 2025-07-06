#include "renderer/renderer_internal.h"


uint32_t _opengl_create_context(struct minec_client* client, bool* vsync_disable_support)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		opengl_loaded = false,
		opengl_context_created = false,
		opengl_context_current = false
	;

	bool glSwapIntervalEXT_support;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_opengl_load() == true) opengl_loaded = true;
		else { minec_client_log_debug_error(client, "window_opengl_load() failed"); result = MINEC_CLIENT_ERROR; }
	}   

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glCreateContext(WINDOW.window_handle, 4, 3, NULL, &RENDERER.BACKEND.device_infos[0].disable_vsync_support) == true) opengl_context_created = true;
		else { minec_client_log_debug_error(client, "window_glCreateContext() with version 4.3 failed"); result = MINEC_CLIENT_ERROR; }
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glMakeCurrent(WINDOW.window_handle)) opengl_context_current = true;
		else { minec_client_log_debug_error(client, "window_glMakeCurrent failed"); result = MINEC_CLIENT_ERROR; }
	}

    if (result == MINEC_CLIENT_SUCCESS) {

        struct load_entry { void** load_dst; uint8_t* func_name; };
#define LOAD_FUNC_ENTRY(func_name) {(void**)&RENDERER.BACKEND.OPENGL.func.func_name, #func_name}

        struct load_entry load_entries[] =
        {
            //general
#ifdef  MINEC_CLIENT_DEBUG_LOG
            LOAD_FUNC_ENTRY(glDebugMessageCallback),
            LOAD_FUNC_ENTRY(glDebugMessageControl),
#endif
            LOAD_FUNC_ENTRY(glEnable),
            LOAD_FUNC_ENTRY(glDisable),
            LOAD_FUNC_ENTRY(glGetError),
            LOAD_FUNC_ENTRY(glGetString),
            LOAD_FUNC_ENTRY(glGetStringi),
            LOAD_FUNC_ENTRY(glGetIntegerv),

            //framebuffer
            LOAD_FUNC_ENTRY(glGenFramebuffers),
            LOAD_FUNC_ENTRY(glDeleteFramebuffers),
            LOAD_FUNC_ENTRY(glBindFramebuffer),
            LOAD_FUNC_ENTRY(glFramebufferTexture2D),
            LOAD_FUNC_ENTRY(glCheckFramebufferStatus),

            //textures
            LOAD_FUNC_ENTRY(glGenTextures),
            LOAD_FUNC_ENTRY(glDeleteTextures),
            LOAD_FUNC_ENTRY(glBindTexture),
            LOAD_FUNC_ENTRY(glTexImage2D),
            LOAD_FUNC_ENTRY(glTexParameteri),

            //render buffer
            LOAD_FUNC_ENTRY(glGenRenderbuffers),
            LOAD_FUNC_ENTRY(glDeleteRenderbuffers),
            LOAD_FUNC_ENTRY(glBindRenderbuffer),
            LOAD_FUNC_ENTRY(glRenderbufferStorage),
            LOAD_FUNC_ENTRY(glFramebufferRenderbuffer),

            //shader
            LOAD_FUNC_ENTRY(glCreateProgram),
            LOAD_FUNC_ENTRY(glDeleteProgram),
            LOAD_FUNC_ENTRY(glCreateShader),
            LOAD_FUNC_ENTRY(glDeleteShader),
            LOAD_FUNC_ENTRY(glShaderSource),
            LOAD_FUNC_ENTRY(glCompileShader),
            LOAD_FUNC_ENTRY(glAttachShader),
            LOAD_FUNC_ENTRY(glLinkProgram),
            LOAD_FUNC_ENTRY(glUseProgram),

            //VAO
            LOAD_FUNC_ENTRY(glGenVertexArrays),
            LOAD_FUNC_ENTRY(glDeleteVertexArrays),
            LOAD_FUNC_ENTRY(glBindVertexArray),
            LOAD_FUNC_ENTRY(glVertexAttribPointer),
            LOAD_FUNC_ENTRY(glEnableVertexAttribArray),
            LOAD_FUNC_ENTRY(glDisableVertexAttribArray),

            //buffer
            LOAD_FUNC_ENTRY(glGenBuffers),
            LOAD_FUNC_ENTRY(glDeleteBuffers),
            LOAD_FUNC_ENTRY(glBindBuffer),
            LOAD_FUNC_ENTRY(glBufferData),
            LOAD_FUNC_ENTRY(glBufferSubData),
            LOAD_FUNC_ENTRY(glMapBuffer),
            LOAD_FUNC_ENTRY(glMapBufferRange),
            LOAD_FUNC_ENTRY(glUnmapBuffer),

            //blend
            LOAD_FUNC_ENTRY(glBlendFunc),
            LOAD_FUNC_ENTRY(glBlendFuncSeparate),
            LOAD_FUNC_ENTRY(glBlendEquation),
            LOAD_FUNC_ENTRY(glBlendEquationSeparate),
            LOAD_FUNC_ENTRY(glBlendColor),

            //stencil
            LOAD_FUNC_ENTRY(glStencilFunc),
            LOAD_FUNC_ENTRY(glStencilOp),
            LOAD_FUNC_ENTRY(glStencilMask),
            LOAD_FUNC_ENTRY(glClearStencil),

            //rendering
            LOAD_FUNC_ENTRY(glViewport),
            LOAD_FUNC_ENTRY(glClear),
            LOAD_FUNC_ENTRY(glClearColor),
            LOAD_FUNC_ENTRY(glDepthFunc),
            LOAD_FUNC_ENTRY(glDepthMask),
            LOAD_FUNC_ENTRY(glCullFace),
            LOAD_FUNC_ENTRY(glFrontFace),

            //draw
            LOAD_FUNC_ENTRY(glDrawArrays),
            LOAD_FUNC_ENTRY(glDrawElements),
            LOAD_FUNC_ENTRY(glDrawArraysInstanced),
            LOAD_FUNC_ENTRY(glDrawElementsInstanced),

        };

        for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*load_entries[i].load_dst = (void*)window_glGetProcAddress(load_entries[i].func_name)) == NULL)
            {
                minec_client_log_debug_error(client, "'window_glGetProcAddress(\"%s\")' failed", load_entries[i].func_name);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    RENDERER.BACKEND.device_count = 0;
    RENDERER.BACKEND.settings.backend_device_index = 0;

    if (RENDERER.BACKEND.settings.vsync == false)
    {
        if (RENDERER.BACKEND.device_infos[0].disable_vsync_support)
        {
            if (window_glSwapIntervalEXT(0) == false)
            {
                RENDERER.BACKEND.settings.vsync = true;
                minec_client_log_debug_error(client, "'window_glSwapIntervalEXT(0)' failed even though its supported");
            }
        }
        else RENDERER.BACKEND.settings.vsync = true;
    }

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (opengl_context_current) if (window_glMakeCurrent(NULL) == false)
		{
			minec_client_log_debug_error(client, "window_glMakeCurrent(NULL) failed");
			client->renderer.state->crashing = true;
			return MINEC_CLIENT_ERROR;
		}
		if (opengl_context_created) if (window_glDestroyContext(WINDOW.window_handle) == false)
		{
			minec_client_log_debug_error(client, "window_glDestroyContext failed");
			client->renderer.state->crashing = true;
			return MINEC_CLIENT_ERROR;
		};
		if (opengl_loaded) window_opengl_unload();
	}

	return result;
}

uint32_t _opengl_destroy_context(struct minec_client* client)
{
	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_debug_error(client, "window_glMakeCurrent(NULL) failed");
		client->renderer.state->crashing = true;
		return MINEC_CLIENT_ERROR;
	}
	if (window_glDestroyContext(WINDOW.window_handle) == false)
	{
		minec_client_log_debug_error(client, "window_glDestroyContext failed");
		client->renderer.state->crashing = true;
		return MINEC_CLIENT_ERROR;
	}
	window_opengl_unload();

	return MINEC_CLIENT_SUCCESS;
}