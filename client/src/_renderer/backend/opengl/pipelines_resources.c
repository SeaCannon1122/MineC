#include "backend_opengl.h"

uint32_t renderer_backend_opengl_pipelines_resources_create(struct minec_client* client, void** backend_base, void** backend_device, void** backend_pipelines_resources, uint32_t pcr_backend_index)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;
    struct renderer_backend_opengl_pipelines_resources* pipelines_resources;

    if (window_glMakeCurrent(client->window.window_handle) == false)
    {
        minec_client_log_debug_error(client, "'window_glMakeCurrent' failed");
        return MINEC_CLIENT_ERROR;
    }

    bool
        pipelines_resources_memory = false
    ;

    if ((pipelines_resources = s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_pipelines_resources))) == NULL)
    {
        minec_client_log_debug_error(client, "'s_alloc(client->static_alloc, sizeof(struct renderer_backend_opengl_pipelines_resources))' failed");
        result = MINEC_CLIENT_ERROR;
    }
    else pipelines_resources_memory = true;
    
    if (result == MINEC_CLIENT_SUCCESS)
    {
        pipelines_resources->pixelchar_renderer.usable = false;
        pipelines_resources->pixelchar_renderer.backend_index = pcr_backend_index;

        uint32_t pixelchar_result;

        if ((pixelchar_result = pixelcharRendererBackendOpenGLInitialize(client->renderer.pixelchar_renderer, pipelines_resources->pixelchar_renderer.backend_index, OPENGL_FRAME_COUNT, window_glGetProcAddress, NULL, 0, NULL, 0)) & PIXELCHAR_ERROR_MASK)
        {
            minec_client_log_error(client, "pixelcharRenderer not usable");
            minec_client_log_debug_error(client, "'pixelcharRendererBackendOpenGLInitialize' failed with '%s'", pixelcharGetResultAsString(pixelchar_result));
        }
        else pipelines_resources->pixelchar_renderer.usable = true;
    }

    if (result != MINEC_CLIENT_SUCCESS)
    {
        if (pipelines_resources_memory)
        {
            if (pipelines_resources->pixelchar_renderer.usable == true) pixelcharRendererBackendOpenGLDeinitialize(client->renderer.pixelchar_renderer, pipelines_resources->pixelchar_renderer.backend_index);
        
            s_free(client->static_alloc, pipelines_resources);;
        }
    }

    if (window_glMakeCurrent(NULL) == false)
    {
        minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
        minec_client_log_debug_error(client, "'window_glMakeCurrent(NULL)' failed");
        minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
    }

    client->renderer.backend.pipelines_resources.pipelines_resources = pipelines_resources;
    return result;
}

void renderer_backend_opengl_pipelines_resources_destroy(struct minec_client* client, void** backend_base, void** backend_device, void** backend_pipelines_resources)
{
    struct renderer_backend_opengl_base* base = client->renderer.backend.base.base;
    struct renderer_backend_opengl_pipelines_resources* pipelines_resources = client->renderer.backend.pipelines_resources.pipelines_resources;

    bool current = true;
    if (window_glMakeCurrent(client->window.window_handle) == false)
    {
        minec_client_log_debug_error(client, "'window_glMakeCurrent' failed");
        current = false;
    }

    if (pipelines_resources->pixelchar_renderer.usable == true)
    {
        if (current) pixelcharRendererBackendOpenGLDeinitialize(client->renderer.pixelchar_renderer, pipelines_resources->pixelchar_renderer.backend_index);
        else pixelcharRendererHardResetBackendSlot(client->renderer.pixelchar_renderer, pipelines_resources->pixelchar_renderer.backend_index);
    }

    s_free(client->static_alloc, pipelines_resources);

    if (window_glMakeCurrent(NULL) == false)
    {
        minec_client_log_error(client, "[FATAL] Failed to unset OpenGL context. Crashing ...");
        minec_client_log_debug_error(client, "'window_glMakeCurrent(NULL)' failed");
        minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
    }
}