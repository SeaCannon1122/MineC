#include "backend_internal.h"

uint32_t renderer_backend_vulkan_base_create(struct minec_client* client, uint32_t slot_index, uint32_t* device_count, uint8_t*** device_infos);
void renderer_backend_vulkan_base_destroy(struct minec_client* client, uint32_t slot_index);

uint32_t renderer_backend_vulkan_device_create(struct minec_client* client, uint32_t slot_index, uint32_t device_index, uint32_t fps);
void renderer_backend_vulkan_device_destroy(struct minec_client* client, uint32_t slot_index);

void renderer_backend_vulkan_set_fps(struct minec_client* client, uint32_t slot_index, uint32_t fps);

uint32_t renderer_backend_vulkan_pipelines_create(struct minec_client* client, uint32_t slot_index);
void renderer_backend_vulkan_pipelines_destroy(struct minec_client* client, uint32_t slot_index);


struct renderer_backend_interface renderer_backend_interfaces[] =
{
#ifdef _MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		renderer_backend_opengl_base_create,
		renderer_backend_opengl_base_destroy,

		renderer_backend_opengl_get_device_infos,

		renderer_backend_opengl_device_create,
		renderer_backend_opengl_device_destroy, 
	},
#endif
#ifdef _MINEC_CLIENT_RENDERER_BACKEND_VULKAN
	{
		renderer_backend_vulkan_base_create,
		renderer_backend_vulkan_base_destroy,

		renderer_backend_vulkan_device_create,
		renderer_backend_vulkan_device_destroy,

		renderer_backend_vulkan_set_fps,

		renderer_backend_vulkan_pipelines_create,
		renderer_backend_vulkan_pipelines_destroy,
	},
#endif
};

void* transfered_window_context;

struct renderer_backend_interface* renderer_backend_get_interfaces(void* window_context, uint32_t* count)
{
	transfered_window_context = window_context;

	*count = sizeof(renderer_backend_interfaces) / sizeof(renderer_backend_interfaces[0]);

	return &renderer_backend_interfaces[0];
}

void* renderer_backend_get_window_context()
{
	return transfered_window_context;
}