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

uint8_t* device_info[1];

uint32_t renderer_backend_opengl_base_create(struct minec_client* client, void** backend_base, uint32_t* device_count, uint8_t*** device_infos)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    bool
        window_context_initialized = false,
        base_memory = false,
        opengl_loaded = false,
        context_created = false,
        extensions_memory = false
    ;

    window_init_context(renderer_backend_get_window_context());

    struct renderer_backend_opengl_base* base;

    if ((base = s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_base))) == NULL)
        result = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
    else
        base_memory = true;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_opengl_load() == false)
        {
            renderer_backend_opengl_log(client, "Failed to load opengl");
            result = MINEC_CLIENT_ERROR;
        }
        else
            opengl_loaded = true;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_glCreateContext(client->window.window_handle, 4, 3, NULL) == false)
        {
            renderer_backend_opengl_log(client, "window_glCreateContext failed");
            result = MINEC_CLIENT_ERROR;
        }
        else context_created = true;
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if (window_glMakeCurrent(client->window.window_handle) == false)
        {
            renderer_backend_opengl_log(client, "window_glMakeCurrent failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS) {

        void** functions[] =
        {
#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG
            (void**)&base->func.glDebugMessageCallback,
            (void**)&base->func.glDebugMessageControl,
#endif
            (void**)&base->func.glEnable,
            (void**)&base->func.glDisable,
            (void**)&base->func.glGetError,
            (void**)&base->func.glGetString,
            (void**)&base->func.glGetStringi,
            (void**)&base->func.glGetIntegerv,

        //framebuffer
            (void**)&base->func.glGenFramebuffers,
            (void**)&base->func.glDeleteFramebuffers,
            (void**)&base->func.glBindFramebuffer,
            (void**)&base->func.glFramebufferTexture2D,
            (void**)&base->func.glCheckFramebufferStatus,

        //textures
            (void**)&base->func.glGenTextures,
            (void**)&base->func.glDeleteTextures,
            (void**)&base->func.glBindTexture,
            (void**)&base->func.glTexImage2D,
            (void**)&base->func.glTexParameteri,

        //render buffer
            (void**)&base->func.glGenRenderbuffers,
            (void**)&base->func.glDeleteRenderbuffers,
            (void**)&base->func.glBindRenderbuffer,
            (void**)&base->func.glRenderbufferStorage,
            (void**)&base->func.glFramebufferRenderbuffer,

        //shader
            (void**)&base->func.glCreateProgram,
            (void**)&base->func.glDeleteProgram,
            (void**)&base->func.glCreateShader,
            (void**)&base->func.glDeleteShader,
            (void**)&base->func.glShaderSource,
            (void**)&base->func.glCompileShader,
            (void**)&base->func.glAttachShader,
            (void**)&base->func.glLinkProgram,
            (void**)&base->func.glUseProgram,

        //VAO
            (void**)&base->func.glGenVertexArrays,
            (void**)&base->func.glDeleteVertexArrays,
            (void**)&base->func.glBindVertexArray,
            (void**)&base->func.glVertexAttribPointer,
            (void**)&base->func.glEnableVertexAttribArray,
            (void**)&base->func.glDisableVertexAttribArray,


        //Buffer
            (void**)&base->func.glGenBuffers,
            (void**)&base->func.glDeleteBuffers,
            (void**)&base->func.glBindBuffer,
            (void**)&base->func.glBufferData,
            (void**)&base->func.glBufferSubData,
            (void**)&base->func.glMapBuffer,
            (void**)&base->func.glMapBufferRange,
            (void**)&base->func.glUnmapBuffer,


        //blend
            (void**)&base->func.glBlendFunc,
            (void**)&base->func.glBlendFuncSeparate,
            (void**)&base->func.glBlendEquation,
            (void**)&base->func.glBlendEquationSeparate,
            (void**)&base->func.glBlendColor,

        //stencil
            (void**)&base->func.glStencilFunc,
            (void**)&base->func.glStencilOp,
            (void**)&base->func.glStencilMask,
            (void**)&base->func.glClearStencil,

            (void**)&base->func.glViewport,
            (void**)&base->func.glClear,
            (void**)&base->func.glClearColor,
            (void**)&base->func.glDepthFunc,
            (void**)&base->func.glDepthMask,
            (void**)&base->func.glCullFace,
            (void**)&base->func.glFrontFace,

        //draw
            (void**)&base->func.glDrawArrays,
            (void**)&base->func.glDrawElements,
            (void**)&base->func.glDrawArraysInstanced,
            (void**)&base->func.glDrawElementsInstanced
        };

        uint8_t* function_names[] =
        {
#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG
            "glDebugMessageCallback",
            "glDebugMessageControl",
#endif
            "glEnable",
            "glDisable",
            "glGetError",
            "glGetString",
            "glGetStringi",
            "glGetIntegerv",

        //framebuffer
            "glGenFramebuffers",
            "glDeleteFramebuffers",
            "glBindFramebuffer",
            "glFramebufferTexture2D",
            "glCheckFramebufferStatus",

        //textures
            "glGenTextures",
            "glDeleteTextures",
            "glBindTexture",
            "glTexImage2D",
            "glTexParameteri",

        //render buffer
            "glGenRenderbuffers",
            "glDeleteRenderbuffers",
            "glBindRenderbuffer",
            "glRenderbufferStorage",
            "glFramebufferRenderbuffer",

        //shader
            "glCreateProgram",
            "glDeleteProgram",
            "glCreateShader",
            "glDeleteShader",
            "glShaderSource",
            "glCompileShader",
            "glAttachShader",
            "glLinkProgram",
            "glUseProgram",

        //VAO
            "glGenVertexArrays",
            "glDeleteVertexArrays",
            "glBindVertexArray",
            "glVertexAttribPointer",
            "glEnableVertexAttribArray",
            "glDisableVertexAttribArray",


        //Buffer
            "glGenBuffers",
            "glDeleteBuffers",
            "glBindBuffer",
            "glBufferData",
            "glBufferSubData",
            "glMapBuffer",
            "glMapBufferRange",
            "glUnmapBuffer",


        //blend
            "glBlendFunc",
            "glBlendFuncSeparate",
            "glBlendEquation",
            "glBlendEquationSeparate",
            "glBlendColor",

        //stencil
            "glStencilFunc",
            "glStencilOp",
            "glStencilMask",
            "glClearStencil",

            "glViewport",
            "glClear",
            "glClearColor",
            "glDepthFunc",
            "glDepthMask",
            "glCullFace",
            "glFrontFace",

        //draw
            "glDrawArrays",
            "glDrawElements",
            "glDrawArraysInstanced",
            "glDrawElementsInstanced"
        };

        for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]) && result == MINEC_CLIENT_SUCCESS; i++)
        {
            if ((*functions[i] = (void*)window_glGetProcAddress(function_names[i])) == NULL)
            {
                renderer_backend_opengl_log(client, "Failed to retrieve '%s'", function_names[i]);
                result = MINEC_CLIENT_ERROR;
            }
        }
    }
    GLenum gl_error;
    while ((gl_error = base->func.glGetError()) != GL_NO_ERROR);


    uint8_t** extensions;
    GLint extension_count = 0;

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);

        if ((result = gl_error_check_log(client, base, "glGetIntegerv with GL_NUM_EXTENSIONS")) == MINEC_CLIENT_SUCCESS)
        {
            if ((extensions = s_alloc(client->dynamic_alloc, sizeof(uint8_t*) * extension_count + 8)) != NULL)
            {
                extensions_memory = true;

                for (GLint i = 0; i < extension_count; i++) if ((extensions[i] = base->func.glGetStringi(GL_EXTENSIONS, i)) == NULL)
                {
                    renderer_backend_opengl_log(client, "glGetStringi(GL_EXTENSIONS, %d) failed", i);
                    result = MINEC_CLIENT_ERROR;
                }
            }
            else result = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
        }
        else result = MINEC_CLIENT_ERROR;
    }

    uint8_t* extension_names[] = {
            "GL_ARB_bindless_texture"
    };

    for (uint32_t i = 0; i < sizeof(extension_names) / sizeof(extension_names[0]) && result == MINEC_CLIENT_SUCCESS; i++)
    {
        bool extension_support = false;

        for (GLint j = 0; j < extension_count && extension_support == false; j++) if (strcmp(extension_names[i], extensions[j]) == NULL) extension_support = true;

        if (extension_support == false)
        {
            renderer_backend_opengl_log(client, "required extension %s not supported", extension_names[i]);
            result = MINEC_CLIENT_ERROR;
        }
    }

#ifdef  MINEC_CLIENT_RENDERER_BACKEND_DEBUG

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glEnable(GL_DEBUG_OUTPUT);
        base->func.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        result = gl_error_check_log(client, base, "Enabeling debug output");
    }
    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glDebugMessageCallback(OpenGLDebugCallback, NULL);
        result = gl_error_check_log(client, base, "Setting debug callback");
    }
    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
        base->func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
        result = gl_error_check_log(client, base, "Configuring debug output");
    }

#endif

    if (result == MINEC_CLIENT_SUCCESS)
    {
        base->func.glEnable(GL_BLEND);
        base->func.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        result = gl_error_check_log(client, base, "Enabeling Blending");
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((device_info[0] = base->func.glGetString(GL_RENDERER)) == NULL)
        {
            renderer_backend_opengl_log(client, "glGetString(GL_RENDERER) failed");
            result = MINEC_CLIENT_ERROR;
        }
    }

    if (result == MINEC_CLIENT_SUCCESS)
    {
        *device_count = 1;
        *device_infos = device_info;
        *backend_base = base;
    }

    window_glMakeCurrent(NULL);
    if (extensions_memory) s_free(client->dynamic_alloc, extensions);
    if (result != MINEC_CLIENT_SUCCESS && context_created) window_glDeleteContext(client->window.window_handle);
    if (result != MINEC_CLIENT_SUCCESS && opengl_loaded) window_opengl_unload();
    if (result != MINEC_CLIENT_SUCCESS && base_memory) s_free(client->static_alloc, base);

    return result;
}

void renderer_backend_opengl_base_destroy(struct minec_client* client, void** backend_base)
{
    struct renderer_backend_opengl_base* base = *backend_base;

    window_glMakeCurrent(NULL);
    window_glDeleteContext(client->window.window_handle);
    window_opengl_unload();
    s_free(client->static_alloc, base);
}