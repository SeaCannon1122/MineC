#include <minec_client.h>

// return value of MINEC_CLIENT_ERROR means after the function call, no backend exists

struct renderer_backend_api
{
	struct renderer_backend_info* (*get_info)(
		struct minec_client* client
	);

	uint32_t(*base_create)(
		struct minec_client* client,
		cwindow_context* window_context,
		cwindow* window,
		struct renderer_backend_device_infos* device_infos,
		struct renderer_backend_base* base
	);

	void (*base_destroy)(
		struct minec_client* client,
		struct renderer_backend_base* base
	);

	uint32_t (*device_create)(
		struct minec_client* client,
		uint32_t device_index,
		struct renderer_backend_device* device
	);

	void (*device_destroy)(
		struct minec_client* client,
		struct renderer_backend_device* device
	);

	uint32_t (*swapchain_create)(
		struct minec_client* client,
		struct renderer_backend_device* device,
		uint32_t width,
		uint32_t height,
		bool vsync,
		bool triple_buffering,
		struct renderer_backend_swapchain* swapchain
	);

	void (*swapchain_destroy)(
		struct minec_client* client,
		struct renderer_backend_device* device,
		struct renderer_backend_swapchain* swapchain
	);

	uint32_t (*frame_start)(
		struct minec_client* client,
		struct renderer_backend_device* device
	);

	uint32_t (*frame_submit)(
		struct minec_client* client,
		struct renderer_backend_device* device
	);
};

#define RENDERER_BACKEND_API(name)\
	{ \
		renderer_backend_##name##_get_info, \
	 \
		renderer_backend_##name##_base_create, \
		renderer_backend_##name##_base_destroy, \
	 \
		renderer_backend_##name##_device_create, \
		renderer_backend_##name##_device_destroy, \
	 \
		renderer_backend_##name##_swapchain_create, \
		renderer_backend_##name##_swapchain_destroy, \
	 \
		renderer_backend_##name##_frame_start, \
		renderer_backend_##name##_frame_submit \
	} \


static struct renderer_backend_api renderer_backend_apis[] = {

#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	RENDERER_BACKEND_API(opengl),
#endif

#ifdef MINEC_CLIENT_RENDERER_BACKEND_VULKAN
	RENDERER_BACKEND_API(vulkan),
#endif

};

struct renderer_backend_info* renderer_backend_get_info(
	struct minec_client* client,
	uint32_t backend_index
)
{
	return renderer_backend_apis[backend_index].get_info(client);
}

uint32_t renderer_backend_base_create(
	struct minec_client* client,
	uint32_t backend_index,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos,
	struct renderer_backend_base* base
)
{
	base->index = backend_index;
	return renderer_backend_apis[backend_index].base_create(client, window_context, window, device_infos, base);
}

void renderer_backend_base_destroy(
	struct minec_client* client,
	struct renderer_backend_base* base
)
{
	renderer_backend_apis[base->index].base_destroy(client, base);
}

uint32_t renderer_backend_device_create(
	struct minec_client* client,
	uint32_t device_index,
	struct renderer_backend_base* base,
	struct renderer_backend_device* device
)
{
	device->base = base;
	return renderer_backend_apis[device->base->index].device_create(client, device_index, device);
}

void renderer_backend_device_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device
)
{
	renderer_backend_apis[device->base->index].device_destroy(client, device);
}

uint32_t renderer_backend_swapchain_create(
	struct minec_client* client,
	struct renderer_backend_device* device,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering,
	struct renderer_backend_swapchain* swapchain
)
{
	return renderer_backend_apis[device->base->index].swapchain_create(client, device, width, height, vsync, triple_buffering, swapchain);
}

void renderer_backend_swapchain_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device,
	struct renderer_backend_swapchain* swapchain
)
{
	renderer_backend_apis[device->base->index].swapchain_destroy(client, device, swapchain);
}

uint32_t renderer_backend_frame_start(
	struct minec_client* client,
	struct renderer_backend_device* device
)
{
	return renderer_backend_apis[device->base->index].frame_start(client, device);
}

uint32_t renderer_backend_frame_submit(
	struct minec_client* client,
	struct renderer_backend_device* device
)
{
	return renderer_backend_apis[device->base->index].frame_submit(client, device);
}