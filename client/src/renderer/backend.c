#include "renderer_internal.h"

struct renderer_backend_api
{
	uint8_t* name;
	uint32_t(*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);
	
	void (*frame)(struct minec_client* client);

	void (*switch_device)(struct minec_client* client, uint32_t device_index);
	void (*reload_resources)(struct minec_client* client);
	void (*set_vsync)(struct minec_client* client, bool vsync);
	void (*set_fps)(struct minec_client* client, uint32_t fps);
	void (*set_max_mipmap_level_count)(struct minec_client* client, uint32_t max_mipmap_level_count);
};

static struct renderer_backend_api renderer_backend_apis[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		"OpenGL",
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy
	},
#endif
};

static const uint32_t RENDERER_BACKEND_COUNT_c = sizeof(renderer_backend_apis) / sizeof(renderer_backend_apis[0]);

uint32_t renderer_backend_create(struct minec_client* client)
{
	struct renderer_backend backend_image;
	backend_image.pixelchar_slots[0] = 0;
	backend_image.pixelchar_slots[1] = 1;
	backend_image.settings.backend_device_index = RENDERER.public.requested_settings.backend_device_index;
	backend_image.settings.fps = RENDERER.public.requested_settings.fps;
	backend_image.settings.vsync = RENDERER.public.requested_settings.vsync;
	backend_image.settings.max_mipmap_level_count = RENDERER.public.requested_settings.max_mipmap_level_count;

	RENDERER.backend_memory_index = 0;	

	for (int32_t i = -1, backend_index = (int32_t)RENDERER.public.requested_settings.backend_index; ; i++, backend_index = i)
	{
		if (backend_index < RENDERER_BACKEND_COUNT_c)
		{
			RENDERER.BACKEND = backend_image;

			if (renderer_backend_apis[backend_index].create(client) == MINEC_CLIENT_SUCCESS)
			{
				RENDERER.public.requested_settings.backend_index = backend_index;
				RENDERER.settings.backend_index = backend_index;
				RENDERER.public.info.state.settings.backend_index = backend_index;
				break;
			}
			else if (RENDERER.crashing) return MINEC_CLIENT_ERROR;
		}
		else if (i == (int32_t)RENDERER_BACKEND_COUNT_c) return MINEC_CLIENT_ERROR;
	}

	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT_c; i++) snprintf(RENDERER.public.info.state.backend.backend_infos[i].name, sizeof(RENDERER.public.info.state.backend.backend_infos[i].name), renderer_backend_apis[i].name);

	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_destroy(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.settings.backend_index].destroy(client);
}

void renderer_backend_frame(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.settings.backend_index].frame(client);
}

void renderer_backend_switch(struct minec_client* client, uint32_t index)
{
	if (index >= RENDERER_BACKEND_COUNT_c) return;
	if (index == RENDERER.settings.backend_index) return;

	struct renderer_backend backend_image = RENDERER.BACKEND;
	if (RENDERER.BACKEND.pixelchar_slots[0] == 0)
	{
		RENDERER.BACKEND.pixelchar_slots[0] = 2;
		RENDERER.BACKEND.pixelchar_slots[1] = 3;
	}
	else
	{
		RENDERER.BACKEND.pixelchar_slots[0] = 0;
		RENDERER.BACKEND.pixelchar_slots[1] = 1;
	}

	if (renderer_backend_apis[index].create(client) != MINEC_CLIENT_SUCCESS) RENDERER.BACKEND = backend_image;
	else
	{
		struct renderer_backend temp_backend_image = RENDERER.BACKEND;
		RENDERER.BACKEND = backend_image;
		renderer_backend_apis[RENDERER.settings.backend_index].destroy(client);
		RENDERER.BACKEND = temp_backend_image;

		RENDERER.public.requested_settings.backend_index = index;
		RENDERER.settings.backend_index = index;

		ACCESS_INFO_STATE(RENDERER.public.info.state.settings.backend_index = index;);
	}
}

void renderer_backend_switch_device(struct minec_client* client, uint32_t device_index)
{
	if (device_index >= RENDERER.BACKEND.device_count);
	renderer_backend_apis[RENDERER.settings.backend_index].switch_device(client, device_index);
}

void renderer_backend_reload_resources(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.settings.backend_index].reload_resources(client);
}

void renderer_backend_set_vsync(struct minec_client* client, bool vsync)
{
	if (vsync = false && RENDERER.BACKEND.device_infos[RENDERER.BACKEND.settings.backend_device_index].disable_vsync_support == false) return;
	renderer_backend_apis[RENDERER.settings.backend_index].set_vsync(client, vsync);
}

void renderer_backend_set_fps(struct minec_client* client, uint32_t fps)
{
	if (fps == 0) return;
	renderer_backend_apis[RENDERER.settings.backend_index].set_fps(client, fps);
}

void renderer_backend_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	if (max_mipmap_level_count == 0) return;
	renderer_backend_apis[RENDERER.settings.backend_index].set_max_mipmap_level_count(client, max_mipmap_level_count);
}