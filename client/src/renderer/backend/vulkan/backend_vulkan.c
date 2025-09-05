#include <minec_client.h>

static struct renderer_backend_info vulkan_info = { .name = "Vulkan", .description = "Might give better performance" };

struct renderer_backend_info* renderer_backend_vulkan_get_info(
	struct minec_client* client
)
{
	return &vulkan_info;
}

uint32_t renderer_backend_vulkan_base_create(
	struct minec_client* client,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos
)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;

	bool
		vulkan_loaded = false,
		instance_created = false
	;

	return result;
}

void renderer_backend_vulkan_base_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_vulkan_device_create(
	struct minec_client* client,
	uint32_t device_index
)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_vulkan_device_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_vulkan_swapchain_create(
	struct minec_client* client,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering
)
{
	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_vulkan_swapchain_destroy(
	struct minec_client* client
)
{

}

uint32_t renderer_backend_vulkan_frame_start(
	struct minec_client* client
)
{
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_vulkan_frame_submit(
	struct minec_client* client
)
{
	return MINEC_CLIENT_SUCCESS;
}