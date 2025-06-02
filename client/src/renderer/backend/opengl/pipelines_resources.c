#include "backend_opengl.h"

uint32_t renderer_backend_opengl_pipelines_resources_create(struct minec_client* client)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;
    struct renderer_backend_opengl_pipelines_resources* pipelines_resources;

    if (window_glMakeCurrent(client->window.window_handle) == false)
    {
        renderer_backend_opengl_log(client, "window_glMakeCurrent failed");
        return MINEC_CLIENT_ERROR;
    }

    bool
        pipelines_resources_memory = false
    ;

    if ((pipelines_resources = s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_pipelines_resources))) == NULL) result = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
    else pipelines_resources_memory = true;
    
    pipelines_resources->pixelchar_renderer.usable = false;
    if (result == MINEC_CLIENT_SUCCESS)
    {
        if ((result = pixelcharRendererBackendOpenGLInitialize(client->renderer.pixelchar_renderer, window_glGetProcAddress, NULL, 0, NULL, 0)) != PIXELCHAR_SUCCESS)
        {
            if (result == PIXELCHAR_ERROR_OUT_OF_MEMORY) result = MINEC_CLIENT_ERROR_OUT_OF_MEMORY;
            else result = MINEC_CLIENT_SUCCESS;
        }
        else
        {
            result = MINEC_CLIENT_SUCCESS;
            pipelines_resources->pixelchar_renderer.usable = true;
        }
    }


    if (result != MINEC_CLIENT_SUCCESS && pipelines_resources->pixelchar_renderer.usable == true) pixelcharRendererBackendOpenGLDeinitialize(client->renderer.pixelchar_renderer);
    if (result != MINEC_CLIENT_SUCCESS && pipelines_resources_memory) s_free(client->static_alloc, pipelines_resources);;

    window_glMakeCurrent(NULL);

    client->renderer.backend.pipelines_resources.pipelines_resources = pipelines_resources;
    return result;
}

void renderer_backend_opengl_pipelines_resources_destroy(struct minec_client* client)
{
    if (window_glMakeCurrent(client->window.window_handle) == false)
    {
        renderer_backend_opengl_log(client, "window_glMakeCurrent failed");
        return MINEC_CLIENT_ERROR;
    }

    struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;
    struct renderer_backend_opengl_pipelines_resources* pipelines_resources = client->renderer.backend.pipelines_resources.pipelines_resources;

    if (pipelines_resources->pixelchar_renderer.usable == true) pixelcharRendererBackendOpenGLDeinitialize(client->renderer.pixelchar_renderer);
    s_free(client->static_alloc, pipelines_resources);

    window_glMakeCurrent(NULL);
}