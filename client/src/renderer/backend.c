#include <minec_client.h>

// return value of MINEC_CLIENT_ERROR means after the function call, no backend exists

struct renderer_backend_api
{
	uint8_t* name;
	uint32_t(*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);
	
	uint32_t(*frame)(struct minec_client* client);

	uint32_t(*switch_device)(struct minec_client* client, uint32_t device_index);
	uint32_t(*reload_resources)(struct minec_client* client);
	uint32_t(*set_vsync)(struct minec_client* client, bool vsync);
	uint32_t(*set_fps)(struct minec_client* client, uint32_t fps);
	uint32_t(*set_max_mipmap_level_count)(struct minec_client* client, uint32_t max_mipmap_level_count);
};

static struct renderer_backend_api renderer_backend_apis[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		"OpenGL",
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy,

		renderer_backend_opengl_frame,

		renderer_backend_opengl_switch_device,
		renderer_backend_opengl_reload_resources,
		renderer_backend_opengl_set_vsync,
		renderer_backend_opengl_set_fps,
		renderer_backend_opengl_set_max_mipmap_level_count,
	},
#endif
};

static const uint32_t RENDERER_BACKEND_COUNT_c = sizeof(renderer_backend_apis) / sizeof(renderer_backend_apis[0]);

uint32_t _renderer_backend_create(struct minec_client* client)
{
	RENDERER.backend.pixelchar_slots[0] = 0;
	RENDERER.backend.pixelchar_slots[1] = 1;
	RENDERER.backend.pixelchar_slot_index = 0;
	
	if (RENDERER.backend.settings.backend_index >= RENDERER_BACKEND_COUNT_c) RENDERER.backend.settings.backend_index = 0;

	for (uint32_t i = UINT32_MAX, backend_index_try = RENDERER.backend.settings.backend_index; ; i++, RENDERER.backend.settings.backend_index = i)
	{
		if (i != backend_index_try)
		{
			if (renderer_backend_apis[RENDERER.backend.settings.backend_index].create(client) == MINEC_CLIENT_SUCCESS) break;
			else if (RENDERER.crashing) return MINEC_CLIENT_ERROR;
		}
		else if (i == (int32_t)RENDERER_BACKEND_COUNT_c) return MINEC_CLIENT_ERROR;
	}
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_create(struct minec_client* client)
{
	RENDERER.backend.settings = RENDERER.public.request.settings.backend;

	if (_renderer_backend_create(client) != MINEC_CLIENT_SUCCESS) return MINEC_CLIENT_ERROR;

	RENDERER.public.info.state.backend.backend_infos.count = RENDERER_BACKEND_COUNT_c;
	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT_c; i++) snprintf(RENDERER.public.info.state.backend.backend_infos.infos[i].name, sizeof(RENDERER.public.info.state.backend.backend_infos.infos[i].name), renderer_backend_apis[i].name);
	RENDERER.public.info.state.backend.backend_device_infos = RENDERER.backend.device_infos;
	RENDERER.public.info.state.settings.backend = RENDERER.backend.settings;
	RENDERER.public.request.settings.backend = RENDERER.backend.settings;

	RENDERER.backend_exists = true;

	return MINEC_CLIENT_SUCCESS;
}

//	renderer_backend_apis[RENDERER.settings.backend_index].reload_resources(client);

void renderer_backend_destroy(struct minec_client* client)
{
	if (RENDERER.backend_exists) renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
}

void renderer_backend_frame(struct minec_client* client)
{
	struct renderer_backend_settings requested_settings;
	RENDERER_ACCESS_REQUEST_STATE(requested_settings = RENDERER.public.request.settings.backend;);
	
	for (uint32_t i = 0; i < 6; i++)
	{
		uint32_t result = MINEC_CLIENT_SUCCESS;

		switch (i)
		{
		case 0:
		{
			if (
				RENDERER.backend.settings.backend_index != requested_settings.backend_index &&
				requested_settings.backend_index < RENDERER_BACKEND_COUNT_c
			)
			{
				if (RENDERER.settings.order_create_new_destroy_old)
				{
					struct renderer_backend backend_image = RENDERER.backend;

					if (RENDERER.backend.pixelchar_slots[0] == 0)
					{
						RENDERER.backend.pixelchar_slots[0] = 2;
						RENDERER.backend.pixelchar_slots[1] = 3;
					}
					else
					{
						RENDERER.backend.pixelchar_slots[0] = 0;
						RENDERER.backend.pixelchar_slots[1] = 1;
					}
					RENDERER.backend.pixelchar_slot_index = 0;

					RENDERER.backend.settings.backend_index = requested_settings.backend_index;
					RENDERER.backend.settings.backend_device_index = 0;

					if (renderer_backend_apis[RENDERER.backend.settings.backend_index].create(client) != MINEC_CLIENT_SUCCESS)
					{
						RENDERER.backend = backend_image;
						if (RENDERER.crashing) return;
					}
					else
					{
						struct renderer_backend temp_backend_image = RENDERER.backend;
						RENDERER.backend = backend_image;

						renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
						RENDERER.backend = temp_backend_image;

						if (RENDERER.crashing) return;
					}
				}
				else
				{
					struct renderer_backend_settings backend_settings = RENDERER.backend.settings;

					renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
					if (RENDERER.crashing) { RENDERER.backend_exists = false; return; }

					RENDERER.backend.settings.backend_index = requested_settings.backend_index;
					RENDERER.backend.settings.backend_device_index = 0;

					result = renderer_backend_apis[RENDERER.backend.settings.backend_index].create(client);
				}
			}
		} break;

		case 1:
		{
			if (RENDERER.backend.settings.backend_device_index != requested_settings.backend_device_index)
				result = renderer_backend_apis[RENDERER.backend.settings.backend_index].switch_device(client, requested_settings.backend_device_index);
		} break;

		case 2:
		{
			if (RENDERER.backend.settings.fps != requested_settings.fps) 
				result = renderer_backend_apis[RENDERER.backend.settings.backend_index].set_fps(client, requested_settings.fps);
		} break;

		case 3:
		{
			if (RENDERER.backend.settings.vsync != requested_settings.vsync)
				result = renderer_backend_apis[RENDERER.backend.settings.backend_index].set_vsync(client, requested_settings.vsync);
		} break;

		case 4:
		{
			if (RENDERER.backend.settings.max_mipmap_level_count != requested_settings.max_mipmap_level_count)
				result = renderer_backend_apis[RENDERER.backend.settings.backend_index].set_max_mipmap_level_count(client, requested_settings.max_mipmap_level_count);
		} break;

		case 5:
		{
			result = renderer_backend_apis[RENDERER.backend.settings.backend_index].frame(client);
		} break;

		default: break;
		}

		if (result != MINEC_CLIENT_SUCCESS)
		{
			if (RENDERER.crashing) { RENDERER.backend_exists = false; return; }

			RENDERER.backend.settings = requested_settings;

			if (_renderer_backend_create(client) != MINEC_CLIENT_SUCCESS)
			{
				RENDERER.crashing = true;
				RENDERER.backend_exists = false;
				return;
			}
		}
	}

	RENDERER_ACCESS_REQUEST_STATE(
		if (RENDERER.public.request.settings.backend.backend_index == requested_settings.backend_index)
			RENDERER.public.request.settings.backend.backend_index = RENDERER.backend.settings.backend_index;

		if (RENDERER.public.request.settings.backend.backend_device_index == requested_settings.backend_device_index)
			RENDERER.public.request.settings.backend.backend_device_index = RENDERER.backend.settings.backend_device_index;

		if (RENDERER.public.request.settings.backend.fps == requested_settings.fps)
			RENDERER.public.request.settings.backend.fps = RENDERER.backend.settings.fps;

		if (RENDERER.public.request.settings.backend.vsync == requested_settings.vsync)
			RENDERER.public.request.settings.backend.vsync = RENDERER.backend.settings.vsync;

		if (RENDERER.public.request.settings.backend.max_mipmap_level_count == requested_settings.max_mipmap_level_count)
			RENDERER.public.request.settings.backend.max_mipmap_level_count = RENDERER.backend.settings.max_mipmap_level_count;
	);
	
	RENDERER_ACCESS_INFO_STATE(
		if (RENDERER.public.info.state.settings.backend.backend_index != RENDERER.backend.settings.backend_index)
		  { RENDERER.public.info.state.settings.backend.backend_index = RENDERER.backend.settings.backend_index; RENDERER.public.info.changed = true; }

		if (RENDERER.public.info.state.settings.backend.backend_device_index != RENDERER.backend.settings.backend_device_index)
		  { RENDERER.public.info.state.settings.backend.backend_device_index = RENDERER.backend.settings.backend_device_index; RENDERER.public.info.changed = true; }

		if (RENDERER.public.info.state.settings.backend.fps != RENDERER.backend.settings.fps)
		  { RENDERER.public.info.state.settings.backend.fps = RENDERER.backend.settings.fps; RENDERER.public.info.changed = true; }

		if (RENDERER.public.info.state.settings.backend.vsync != RENDERER.backend.settings.vsync)
		  { RENDERER.public.info.state.settings.backend.vsync = RENDERER.backend.settings.vsync; RENDERER.public.info.changed = true; }

		if (RENDERER.public.info.state.settings.backend.max_mipmap_level_count != RENDERER.backend.settings.max_mipmap_level_count)
		  { RENDERER.public.info.state.settings.backend.max_mipmap_level_count = RENDERER.backend.settings.max_mipmap_level_count; RENDERER.public.info.changed = true; }

		if (RENDERER.backend.device_infos_changed)
		{
			RENDERER.public.info.state.backend.backend_device_infos = RENDERER.backend.device_infos;
			RENDERER.public.info.changed = true;
			RENDERER.backend.device_infos_changed = false;
		}
	);

}