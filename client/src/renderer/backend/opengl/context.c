#include <minec_client.h>

uint32_t _context_create(struct minec_client* client)
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
		else { minec_client_log_debug_l(client, "window_opengl_load() failed"); result = MINEC_CLIENT_ERROR; }
	}   

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glCreateContext(APPLICATION_WINDOW.window_handle, 4, 3, NULL, &RENDERER.backend.device_infos.infos[0].disable_vsync_support) == true) opengl_context_created = true;
		else { minec_client_log_debug_l(client, "window_glCreateContext() with version 4.3 failed"); result = MINEC_CLIENT_ERROR; }

        if (RENDERER.backend.device_infos.infos[0].disable_vsync_support == false)
        {
            minec_client_log_debug(client, "Vsync disabeling not supported");
            result = MINEC_CLIENT_ERROR;
        }
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (window_glMakeCurrent(APPLICATION_WINDOW.window_handle)) opengl_context_current = true;
		else { minec_client_log_debug_l(client, "window_glMakeCurrent failed"); result = MINEC_CLIENT_ERROR; }
	}

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_glSwapIntervalEXT(1) == false)
        {
            minec_client_log_debug_l(client, "'window_glSwapIntervalEXT(1)' failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        struct load_entry { void** load_dst; uint8_t* func_name; };

        struct load_entry load_entries[] =
        {
#define OPENGL_FUNCTION(signature, name) {(void**)&OPENGL.func.name, #name},
        OPENGL_FUNCTION_LIST
#undef OPENGL_FUNCTION
        };

        for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*load_entries[i].load_dst = (void*)window_glGetProcAddress(load_entries[i].func_name)) == NULL)
            {
                minec_client_log_debug_l(client, "'window_glGetProcAddress(\"%s\")' failed", load_entries[i].func_name);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        opengl_errors_clear(client);
        OPENGL.func.glGetIntegerv(GL_NUM_EXTENSIONS, &OPENGL.extension_count);

        if (opengl_error_get_log(client, "glGetIntegerv with GL_NUM_EXTENSIONS")) result = MINEC_CLIENT_ERROR;

        if (OPENGL.extension_count > sizeof(OPENGL.extensions) / sizeof(OPENGL.extensions[0]))
            OPENGL.extension_count = sizeof(OPENGL.extensions) / sizeof(OPENGL.extensions[0]);
        if (OPENGL.extension_count < 0)
            OPENGL.extension_count = 0;
    }

    for (GLint i = 0; i < OPENGL.extension_count && result == MINEC_CLIENT_SUCCESS; i++)
    {
        if ((OPENGL.extensions[i] = (GLubyte*)OPENGL.func.glGetStringi(GL_EXTENSIONS, i)) == NULL)
        {
            minec_client_log_debug_l(client, "'glGetStringi(GL_EXTENSIONS, %d)' failed", i);
            result = MINEC_CLIENT_ERROR;
        }
    }

    uint8_t* extension_names[] = {
        "GL_ARB_bindless_texture"
    };

    for (uint32_t i = 0; i < sizeof(extension_names) / sizeof(extension_names[0]) && result == MINEC_CLIENT_SUCCESS; i++)
    {
        bool extension_support = false;
        for (GLint j = 0; j < OPENGL.extension_count && extension_support == false; j++) if (strcmp(extension_names[i], OPENGL.extensions[j]) == 0) extension_support = true;

        if (extension_support == false)
        {
            minec_client_log_error(client, "[RENDERER] [OPENGL] Extension %s not supported", extension_names[i]);
            result = MINEC_CLIENT_ERROR;
        }
    }
    
    if (result == MINEC_CLIENT_SUCCESS)
    {
        GLubyte* name;
        GLubyte* version;

        if ((name = (GLubyte*)OPENGL.func.glGetString(GL_RENDERER)) == NULL || (version = (GLubyte*)OPENGL.func.glGetString(GL_VERSION)) == NULL) result = MINEC_CLIENT_ERROR;
        else
        {
            snprintf(RENDERER.backend.device_infos.infos[0].name, sizeof(RENDERER.backend.device_infos.infos[0].name), name);
            snprintf(RENDERER.backend.device_infos.infos[0].name, sizeof(RENDERER.backend.device_infos.infos[0].version), version);
        }
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        RENDERER.backend.device_infos.count = 1;
        RENDERER.backend.settings.backend_device_index = 0;
        RENDERER.backend.device_infos.infos[0].usable = true;
        OPENGL.last_frame_time = 0.f;
        OPENGL.resource_frame_index = 0;

        if (RENDERER.backend.settings.vsync == false) if (window_glSwapIntervalEXT(0) == false)
        {
            RENDERER.backend.settings.vsync = true;
            minec_client_log_debug_l(client, "'window_glSwapIntervalEXT(0)' failed");
        }
    }

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (opengl_context_current) if (window_glMakeCurrent(NULL) == false)
		{
			minec_client_log_debug_l(client, "window_glMakeCurrent(NULL) failed");
            RENDERER_CRASHING = true;
			return MINEC_CLIENT_ERROR;
		}
		if (opengl_context_created) if (window_glDestroyContext(APPLICATION_WINDOW.window_handle) == false)
		{
			minec_client_log_debug_l(client, "window_glDestroyContext failed");
            RENDERER_CRASHING = true;
			return MINEC_CLIENT_ERROR;
		};
		if (opengl_loaded) window_opengl_unload();
	}

	return result;
}

void _context_destroy(struct minec_client* client)
{
	if (window_glMakeCurrent(NULL) == false)
	{
		minec_client_log_debug_l(client, "window_glMakeCurrent(NULL) failed");
		RENDERER_CRASHING = true;
        return;
	}
	if (window_glDestroyContext(APPLICATION_WINDOW.window_handle) == false)
	{
		minec_client_log_debug_l(client, "window_glDestroyContext failed");
        RENDERER_CRASHING = true;
		return;
	}
	window_opengl_unload();

	return;
}

uint32_t _context_frame_submit(struct minec_client* client)
{
    if (window_glSwapBuffers(APPLICATION_WINDOW.window_handle) == false)
    {
        minec_client_log_debug_l(client, "window_glSwapBuffers failed");
        opengl_errors_clear(client);
    }

    if (RENDERER.backend.settings.fps != 0)
    {
        float frame_time_delta = time_get() - OPENGL.last_frame_time;
        float sleep_time = 1000.f / RENDERER.backend.settings.fps - frame_time_delta;

        if (sleep_time > 0.f) time_sleep(sleep_time > 1000.f ? 1000.f : sleep_time);
    }
    
    OPENGL.last_frame_time = time_get();
    OPENGL.resource_frame_index = (OPENGL.resource_frame_index + 1) % OPENGL_RESOURCE_FRAME_COUNT;

    return MINEC_CLIENT_SUCCESS;
}