#include "backend_opengl.h"

void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam) {
    // Output all debug messages (info, warnings, errors)
    printf("\n\n----OpenGl-Debug-Message-----------------\n");
    printf("  Source: %u\n", source);
    printf("  Type: %u\n", type);
    printf("  ID: %u\n", id);
    printf("  Severity: %u\n", severity);
    printf("  Message: %s\n", message);
    printf("-----------------------------------------\n\n");
}

uint32_t renderer_backend_opengl_base_create(struct minec_client* client, void** backend_base, uint32_t* device_count, uint8_t*** device_infos)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    bool
        cwindow_context_initialized = false,
        base_memory = false,
        opengl_loaded = false,
        context_created = false,
        extensions_memory = false
    ;

    bool currrent = false;

    cwindow_init_context(renderer_backend_get_window_context());

    struct renderer_backend_opengl_base* base;

    if ((base = s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_base))) == NULL)
    {
        minec_client_log_debug_l(client, "'s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_base))' failed");
        result = MINEC_CLIENT_ERROR;
    }
    else base_memory = true;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_opengl_load() == false)
        {
            minec_client_log_debug_l(client, "'window_opengl_load()' failed");
            result = MINEC_CLIENT_ERROR;
        }
        else opengl_loaded = true;
    }
    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_glCreateContext(client->window.window, 4, 3, NULL, &base->vsync_support) == false)
        {
            minec_client_log_debug_l(client, "'window_glCreateContext(client->window.window, 4, 3, NULL)' failed");
            result = MINEC_CLIENT_ERROR;
        }
        else context_created = true;
    }
    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_glMakeCurrent(client->window.window) == false)
        {
            minec_client_log_debug_l(client, "'window_glMakeCurrent' failed");
            result = MINEC_CLIENT_ERROR;
        }
        else currrent = true;
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        struct load_entry { void** load_dst; uint8_t* func_name; };
#define LOAD_FUNC_ENTRY(func_name) {(void**)&base->func.func_name, #func_name}

        struct load_entry load_entries[] =
        {
            //general
#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG
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
                minec_client_log_debug_l(client, "'window_glGetProcAddress(\"%s\")' failed", load_entries[i].func_name);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        minec_client_retrieve_log_opengl_errors(client, base, &result, "Initial error reset");
        result = MINEC_CLIENT_SUCCESS;
    } minec_client_retrieve_log_opengl_errors(client, base, &result, "Initial error reset");
    
    uint8_t** extensions;
    GLint extension_count = 0;

    if (result == MINEC_CLIENT_SUCCESS) base->func.glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    minec_client_retrieve_log_opengl_errors(client, base, &result, "glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count)");

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((extensions = s_alloc(client->dynamic_alloc, sizeof(uint8_t*) * extension_count + 8)) == NULL)
        {
            minec_client_log_debug_l(client, "'s_alloc(client->dynamic_alloc, sizeof(uint8_t*) * extension_count(%d) + 8)' failed", extension_count);
            result = MINEC_CLIENT_ERROR;
        }
        else extensions_memory = true;
    }
    for (GLint i = 0; result == MINEC_CLIENT_SUCCESS && i < extension_count; i++) if ((extensions[i] = base->func.glGetStringi(GL_EXTENSIONS, i)) == NULL)
    {
        minec_client_log_debug_l(client, "'glGetStringi(GL_EXTENSIONS, %d)' failed", i);
        result = MINEC_CLIENT_ERROR;
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
            minec_client_log_debug_l(client, "required extension %s not supported", extension_names[i]);
            result = MINEC_CLIENT_ERROR;
        }
    }

#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glEnable(GL_DEBUG_OUTPUT);
        base->func.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        base->func.glDebugMessageCallback(OpenGLDebugCallback, NULL);

        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    }
    minec_client_retrieve_log_opengl_errors(client, base, &result, "Configuring OpenGL debug output");

#endif

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glEnable(GL_BLEND);
        base->func.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    minec_client_retrieve_log_opengl_errors(client, base, &result, "Configuring blending");

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((base->device_info[0] = base->func.glGetString(GL_RENDERER)) == NULL)
        {
            minec_client_log_debug_l(client, "'glGetString(GL_RENDERER)' failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        *device_count = 1;
        *device_infos = base->device_info;
        *backend_base = base;
    }
    
    if (currrent) if (window_glMakeCurrent(NULL) == false)
    {
        minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
        minec_client_log_debug_l(client, "'window_glMakeCurrent(NULL)' failed");
        minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
    }

    if (extensions_memory) s_free(client->dynamic_alloc, extensions);
    if (result != MINEC_CLIENT_SUCCESS && context_created) cwindow_glDestroyContext(client->window.window);
    if (result != MINEC_CLIENT_SUCCESS && opengl_loaded) cwindow_opengl_unload();
    if (result != MINEC_CLIENT_SUCCESS && base_memory) s_free(client->static_alloc, base);

    return result;
}

void renderer_backend_opengl_base_destroy(struct minec_client* client, void** backend_base)
{
    struct renderer_backend_opengl_base* base = *backend_base;

    cwindow_glDestroyContext(client->window.window);
    cwindow_opengl_unload();
    s_free(client->static_alloc, base);
}