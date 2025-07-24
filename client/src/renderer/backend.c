#include "renderer_internal.h"

struct renderer_backend_api
{
	uint32_t(*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);
};

static uint8_t* renderer_backend_names[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	"OpenGL",
#endif
};

static struct renderer_backend_api renderer_backend_apis[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy
	},
#endif
};

static const uint32_t RENDERER_BACKEND_COUNT_c = sizeof(renderer_backend_names) / sizeof(renderer_backend_names[0]);

uint32_t renderer_backend_create(struct minec_client* client)
{
	RENDERER.backend_memory_index = 0;

	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT_c; i++) snprintf(RENDERER.public.info.state.backend.backend_infos[i].name, sizeof(RENDERER.public.info.state.backend.backend_infos[i].name), renderer_backend_names[i]);

	memcpy(RENDERER.public.info.state.backend.backend_device_infos, RENDERER.BACKEND.device_infos, sizeof(RENDERER.BACKEND.device_infos));
	RENDERER.BACKEND.settings.backend_device_index = RENDERER.public.requested_settings.backend_device_index;
	RENDERER.BACKEND.settings.fps = RENDERER.public.requested_settings.fps;
	RENDERER.BACKEND.settings.vsync = RENDERER.public.requested_settings.vsync;
	RENDERER.BACKEND.settings.max_mipmap_level_count = RENDERER.public.requested_settings.max_mipmap_level_count;


	for (int32_t i = -1, backend_index = (int32_t)RENDERER.public.requested_settings.backend_index; ;i++, backend_index = i)
	{
		if (backend_index < RENDERER_BACKEND_COUNT_c)
		{
			if (renderer_backend_apis[backend_index].create(client) == MINEC_CLIENT_SUCCESS)
			{
				RENDERER.public.requested_settings.backend_index = backend_index;
				RENDERER.settings.backend_index = backend_index;
				break;
			}
			else if (RENDERER.crashing) return MINEC_CLIENT_ERROR;
		}
		else if (i == (int32_t)RENDERER_BACKEND_COUNT_c) return MINEC_CLIENT_ERROR;
	}

	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_destroy(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.settings.backend_index].destroy(client);
}