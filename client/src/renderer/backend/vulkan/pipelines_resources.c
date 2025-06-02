#include "backend_vulkan.h"

#include <pixelchar/backend/backend_vulkan.h>

uint32_t renderer_backend_vulkan_pipelines_resources_create(struct minec_client* client)
{
    uint32_t result = MINEC_CLIENT_SUCCESS;

    struct renderer_backend_vulkan_base* base = client->renderer.backend.base.base;
    struct renderer_backend_vulkan_device* device = client->renderer.backend.device.device;
    struct renderer_backend_vulkan_pipelines_resources* pipelines_resources;

    if ((client->renderer.backend.pipelines_resources.pipelines_resources = s_alloc(client->static_alloc, sizeof(struct renderer_backend_vulkan_pipelines_resources))) == NULL) return MINEC_CLIENT_ERROR_OUT_OF_MEMORY;

    pipelines_resources = client->renderer.backend.pipelines_resources.pipelines_resources;

    /*if ((result = pixelcharRendererBackendVulkanInitialize(
        client->renderer.pixelchar_renderer,
        device->device,
        base->physical_devices[device->physical_device_index],
        device->queue,
        device->queue_family_index,
        NULL,
        base->func.vkGetDeviceProcAddr,
        NULL, 0, NULL , 0
    )) == PIXELCHAR_SUCCESS) pipelines_resources->pixelchar_renderer.usable = true;*/

    return result;
}

void renderer_backend_vulkan_pipelines_resources_destroy(struct minec_client* client)
{
    struct renderer_backend_vulkan_base* base = client->renderer.backend.base.base;
    struct renderer_backend_vulkan_device* device = client->renderer.backend.device.device;
    struct renderer_backend_vulkan_pipelines_resources* pipelines_resources = client->renderer.backend.pipelines_resources.pipelines_resources;

    s_free(client->static_alloc, pipelines_resources);
}