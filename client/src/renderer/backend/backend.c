#include <minec_client.h>

#define RENDERER_BACKEND_COUNT RENDERER_BACKEND_COUNTER_OPENGL + RENDERER_BACKEND_COUNTER_VULKAN

struct renderer_backend_api
{
	struct renderer_backend_info* (*get_info)(
		struct minec_client* client
	);

	uint32_t(*base_create)(
		struct minec_client* client,
		cwindow_context* window_context,
		cwindow* window,
		struct renderer_backend_device_infos* device_infos
	);

	void (*base_destroy)(
		struct minec_client* client
	);

	uint32_t (*device_create)(
		struct minec_client* client,
		uint32_t device_index
	);

	void (*device_destroy)(
		struct minec_client* client
	);

	uint32_t (*swapchain_create)(
		struct minec_client* client,
		uint32_t width,
		uint32_t height,
		bool vsync,
		bool triple_buffering
	);

	void (*swapchain_destroy)(
		struct minec_client* client
	);

	uint32_t (*frame_start)(
		struct minec_client* client
	);

	uint32_t (*frame_submit)(
		struct minec_client* client
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

void renderer_backend_get_infos(
	struct minec_client* client,
	struct renderer_backend_infos* infos
)
{
	infos->count = RENDERER_BACKEND_COUNT;
	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT; i++) infos->infos[i] = *renderer_backend_apis[i].get_info(client);
}

uint32_t renderer_backend_base_create(
	struct minec_client* client,
	uint32_t backend_index,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos
)
{
	RENDERER.components.backend.index = backend_index;
	return renderer_backend_apis[RENDERER.components.backend.index].base_create(client, window_context, window, device_infos);
}

void renderer_backend_base_destroy(
	struct minec_client* client
)
{
	renderer_backend_apis[RENDERER.components.backend.index].base_destroy(client);
}

uint32_t renderer_backend_device_create(
	struct minec_client* client,
	uint32_t device_index
)
{
	return renderer_backend_apis[RENDERER.components.backend.index].device_create(client, device_index);
}

void renderer_backend_device_destroy(
	struct minec_client* client
)
{
	renderer_backend_apis[RENDERER.components.backend.index].device_destroy(client);
}

uint32_t renderer_backend_swapchain_create(
	struct minec_client* client,
	uint32_t width,
	uint32_t height,
	bool vsync,
	bool triple_buffering
)
{
	return renderer_backend_apis[RENDERER.components.backend.index].swapchain_create(client, width, height, vsync, triple_buffering);
}

void renderer_backend_swapchain_destroy(
	struct minec_client* client
)
{
	renderer_backend_apis[RENDERER.components.backend.index].swapchain_destroy(client);
}

uint32_t renderer_backend_frame_start(
	struct minec_client* client
)
{
	return renderer_backend_apis[RENDERER.components.backend.index].frame_start(client);
}

uint32_t renderer_backend_frame_submit(
	struct minec_client* client
)
{
	return renderer_backend_apis[RENDERER.components.backend.index].frame_submit(client);
}