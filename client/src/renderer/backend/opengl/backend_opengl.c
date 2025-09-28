#include <minec_client.h>

bool _opengl_error_get_log(struct minec_client* client, uint8_t* action, uint8_t* function, uint8_t* file, uint32_t line)
{
	GLenum error = OPENGL_FUNC.glGetError();
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
	for (uint32_t i = 0; OPENGL_FUNC.glGetError() != GL_NO_ERROR; i++)
		if (i > 64) return MINEC_CLIENT_ERROR;
	return MINEC_CLIENT_SUCCESS;
}

#ifdef MINEC_CLIENT_DEBUG_LOG

static void DebugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
)
{
    struct minec_client* client = (struct minec_client*)userParam;

    minec_client_log_debug(client, "--OpenGL-Debug-Message-- Source: %u | Type : %u | ID : %u | Severity : %u | Message : %s", source, type, id, severity, message);
}

#endif

struct load_entry { void** load_dst; uint8_t* func_name; };

#define OPENGL_FUNCTION(signature, name) {(void**)&OPENGL_FUNC.name, #name},

static struct renderer_backend_info opengl_info = { .name = "OpenGL", .description = "Might be unstable" };

struct renderer_backend_info* renderer_backend_opengl_get_info(
	struct minec_client* client
)
{
	return &opengl_info;
}

uint32_t renderer_backend_opengl_base_create(
	struct minec_client* client,
    cwindow_context* window_context,
    cwindow* window,
	struct renderer_backend_device_infos* device_infos
)
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
        if (cwindow_context_graphics_opengl_load(window_context) == true) opengl_loaded = true;
        else { minec_client_log_debug_l(client, "cwindow_context_graphics_opengl_load() failed"); result = MINEC_CLIENT_ERROR; }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        bool glSwapIntervalEXT_suopport = false;

        if (cwindow_glCreateContext(window, 4, 3, NULL, &glSwapIntervalEXT_suopport) == true) opengl_context_created = true;
        else { minec_client_log_debug_l(client, "window_glCreateContext() with version 4.3 failed"); result = MINEC_CLIENT_ERROR; }

        if (glSwapIntervalEXT_suopport == false)
        {
            minec_client_log_error(client, "[RENDERER] [OPENGL] 'glSwapIntervalEXT' not supported");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (cwindow_glMakeCurrent(window, true)) opengl_context_current = true;
        else { minec_client_log_debug_l(client, "window_glMakeCurrent failed"); result = MINEC_CLIENT_ERROR; }
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        struct load_entry load_entries[] = { OPENGL_FUNCTION_LIST };

        for (uint32_t i = 0; i < sizeof(load_entries) / sizeof(load_entries[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*load_entries[i].load_dst = (void*)cwindow_glGetProcAddress(window, load_entries[i].func_name)) == NULL)
            {
                minec_client_log_debug_l(client, "'window_glGetProcAddress(\"%s\")' failed", load_entries[i].func_name);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    GLubyte* extensions[1024];
    GLint extension_count = 0;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        opengl_errors_clear(client);
        OPENGL_FUNC.glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);

        if (opengl_error_get_log(client, "glGetIntegerv with GL_NUM_EXTENSIONS")) result = MINEC_CLIENT_ERROR;

        if (extension_count > sizeof(extensions) / sizeof(extensions[0]))
            extension_count = sizeof(extensions) / sizeof(extensions[0]);
        if (extension_count < 0)
            extension_count = 0;
    }

    for (GLint i = 0; i < extension_count && result == MINEC_CLIENT_SUCCESS; i++)
    {
        if ((extensions[i] = (GLubyte*)OPENGL_FUNC.glGetStringi(GL_EXTENSIONS, i)) == NULL)
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
        for (GLint j = 0; j < extension_count && extension_support == false; j++) if (strcmp(extension_names[i], extensions[j]) == 0) extension_support = true;

        if (extension_support == false)
        {
            minec_client_log_error(client, "[RENDERER] [OPENGL] Extension %s not supported", extension_names[i]);
            result = MINEC_CLIENT_ERROR;
        }
    }

#ifdef MINEC_CLIENT_DEBUG_LOG

    if (result == MINEC_CLIENT_SUCCESS)
    {
        OPENGL_FUNC.glEnable(GL_DEBUG_OUTPUT);
        OPENGL_FUNC.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        OPENGL_FUNC.glDebugMessageCallback((GLDEBUGPROC)DebugCallback, (const void*)client);

        OPENGL_FUNC.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
        OPENGL_FUNC.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
        OPENGL_FUNC.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
        OPENGL_FUNC.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

        if (opengl_error_get_log(client, "Initializing debug messenger")) result = MINEC_CLIENT_ERROR;
    }
#endif

    if (result == MINEC_CLIENT_SUCCESS)
    {
        GLubyte* name;
        GLubyte* version;

        if ((name = (GLubyte*)OPENGL_FUNC.glGetString(GL_RENDERER)) == NULL || (version = (GLubyte*)OPENGL_FUNC.glGetString(GL_VERSION)) == NULL) result = MINEC_CLIENT_ERROR;
        else
        {
            snprintf(device_infos->infos[0].name, sizeof(device_infos->infos[0].name), name);
            snprintf(device_infos->infos[0].description, sizeof(device_infos->infos[0].description), version);
        }
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        device_infos->count = 1;
        device_infos->infos[0].usable = true;
        device_infos->infos[0].disable_vsync_support = true;
        device_infos->infos[0].triple_buffering_support = false;
        OPENGL.window_context = window_context;
        OPENGL.window = window;
        OPENGL.resource_frame_index = 0;
    }
    else
    {
        if (opengl_context_current) cwindow_glMakeCurrent(window, false);
        if (opengl_context_created) cwindow_glDestroyContext(window);
        if (opengl_loaded) cwindow_context_graphics_opengl_unload(window_context);
    }

    return result;
}

void renderer_backend_opengl_base_destroy(
	struct minec_client* client
)
{
    cwindow_glMakeCurrent(OPENGL.window, false);
    cwindow_glDestroyContext(OPENGL.window);
    cwindow_context_graphics_opengl_unload(OPENGL.window_context);
}

uint32_t renderer_backend_opengl_device_create(
	struct minec_client* client,
	uint32_t device_index
)
{
    return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_opengl_device_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_opengl_swapchain_create(
	struct minec_client* client,
    uint32_t width,
    uint32_t height,
	bool vsync,
    bool triple_buffering
) 
{
    OPENGL_FUNC.glViewport(0, 0, width, height);

    OPENGL.swapchain.width = width;
    OPENGL.swapchain.height = height;

    if (cwindow_glSwapIntervalEXT(OPENGL.window, vsync ? 1 : 0)) return MINEC_CLIENT_SUCCESS;
    else
    {
        minec_client_log_debug_l(client, vsync ? "'window_glSwapIntervalEXT(1)' failed" : "'window_glSwapIntervalEXT(0)' failed");
        return MINEC_CLIENT_ERROR;
    }
}

void renderer_backend_opengl_swapchain_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_opengl_frame_start(
    struct minec_client* client
)
{
    OPENGL_FUNC.glClear(GL_COLOR_BUFFER_BIT);

    return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_opengl_frame_submit(
    struct minec_client* client
)
{
    OPENGL.resource_frame_index = (OPENGL.resource_frame_index + 1) % OPENGL_RESOURCE_FRAME_COUNT;

    if (cwindow_glSwapBuffers(OPENGL.window) == false)
    {
        minec_client_log_debug_l(client, "window_glSwapBuffers failed");
        opengl_errors_clear(client);
        return MINEC_CLIENT_ERROR;
    }

    return MINEC_CLIENT_SUCCESS;
}