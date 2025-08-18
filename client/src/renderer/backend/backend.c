#include <minec_client.h>

// return value of MINEC_CLIENT_ERROR means after the function call, no backend exists

struct renderer_backend_api
{
	uint8_t* name;
	uint32_t(*create)(struct minec_client* client);
	void (*destroy)(struct minec_client* client);

	uint32_t(*reload_assets)(struct minec_client* client);

	uint32_t(*switch_device)(struct minec_client* client, uint32_t device_index);
	uint32_t(*set_vsync)(struct minec_client* client, bool vsync);
	uint32_t(*set_fps)(struct minec_client* client, uint32_t fps);
	uint32_t(*set_max_mipmap_level_count)(struct minec_client* client, uint32_t max_mipmap_level_count);

	uint32_t(*frame_begin)(struct minec_client* client);
	uint32_t(*frame_menu)(struct minec_client* client);
	uint32_t(*frame_end)(struct minec_client* client);
};

static struct renderer_backend_api renderer_backend_apis[] = {
#ifdef MINEC_CLIENT_RENDERER_BACKEND_OPENGL
	{
		"OpenGL",
		renderer_backend_opengl_create,
		renderer_backend_opengl_destroy,

		renderer_backend_opengl_reload_assets,

		renderer_backend_opengl_switch_device,
		renderer_backend_opengl_set_vsync,
		renderer_backend_opengl_set_fps,
		renderer_backend_opengl_set_max_mipmap_level_count,

		renderer_backend_opengl_frame_begin,
		renderer_backend_opengl_frame_menu,
		renderer_backend_opengl_frame_end
	},
#endif
};

static const uint32_t RENDERER_BACKEND_COUNT_c = sizeof(renderer_backend_apis) / sizeof(renderer_backend_apis[0]);

uint32_t _renderer_backend_create(struct minec_client* client)
{
	RENDERER.backend.pixelchar_slot = 0;
	RENDERER.backend.pixelchar_slot_offset = 0;
	
	if (RENDERER.backend.settings.backend_index >= RENDERER_BACKEND_COUNT_c) RENDERER.backend.settings.backend_index = 0;

	struct renderer_backend_settings settings = RENDERER.backend.settings;

	for (uint32_t i = UINT32_MAX, backend_index_try = RENDERER.backend.settings.backend_index; ; i++, RENDERER.backend.settings = settings, RENDERER.backend.settings.backend_index = i)
	{
		if (i != backend_index_try)
		{
			if (renderer_backend_apis[RENDERER.backend.settings.backend_index].create(client) == MINEC_CLIENT_SUCCESS) break;
			else if (RENDERER_CRASHING) return MINEC_CLIENT_ERROR;
		}
		else if (i == (int32_t)RENDERER_BACKEND_COUNT_c) return MINEC_CLIENT_ERROR;
	}
	return MINEC_CLIENT_SUCCESS;
}

uint32_t renderer_backend_create(struct minec_client* client)
{
	if (_renderer_backend_create(client) != MINEC_CLIENT_SUCCESS) return MINEC_CLIENT_ERROR;

	RENDERER.public.state.info.backend_infos.count = RENDERER_BACKEND_COUNT_c;
	for (uint32_t i = 0; i < RENDERER_BACKEND_COUNT_c; i++) snprintf(RENDERER.public.state.info.backend_infos.infos[i].name, sizeof(RENDERER.public.state.info.backend_infos.infos[i].name), renderer_backend_apis[i].name);
	RENDERER.public.state.info.backend_device_infos = RENDERER.backend.device_infos;

	RENDERER_BACKEND_EXISTS = true;

	return MINEC_CLIENT_SUCCESS;
}

void renderer_backend_destroy(struct minec_client* client)
{
	if (RENDERER_BACKEND_EXISTS == false) return; 
	renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
}

void renderer_backend_switch(struct minec_client* client, uint32_t index)
{
	if ( RENDERER.backend.settings.backend_index != index && index < RENDERER_BACKEND_COUNT_c )
	{
		if (RENDERER.settings.order_create_new_destroy_old)
		{
			struct renderer_backend backend_image = RENDERER.backend;

			if (RENDERER.backend.pixelchar_slot == 0) RENDERER.backend.pixelchar_slot = 2;
			else RENDERER.backend.pixelchar_slot = 0;

			RENDERER.backend.settings.backend_index = index;
			RENDERER.backend.settings.backend_device_index = 0;

			if (renderer_backend_apis[RENDERER.backend.settings.backend_index].create(client) != MINEC_CLIENT_SUCCESS) RENDERER.backend = backend_image;
			else
			{
				struct renderer_backend temp_backend_image = RENDERER.backend;
				RENDERER.backend = backend_image;

				renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
				RENDERER.backend = temp_backend_image;
			}
		}
		else
		{
			renderer_backend_apis[RENDERER.backend.settings.backend_index].destroy(client);
			if (RENDERER_CRASHING) { RENDERER_BACKEND_EXISTS = false; return; }

			RENDERER.backend.settings.backend_index = index;
			RENDERER.backend.settings.backend_device_index = 0;

			if (_renderer_backend_create(client) != MINEC_CLIENT_SUCCESS) { RENDERER_BACKEND_EXISTS = false; RENDERER_CRASHING = true; }
		}
	}
}

#define RENDERER_BACKEND_SETTING_ACTION_FUNCTION(declaration, condition, call)\
declaration\
{\
	if (condition)\
	{\
		struct renderer_backend_settings settings = RENDERER.backend.settings;\
	\
		if (call != MINEC_CLIENT_SUCCESS)\
		{\
			if (RENDERER_CRASHING) { RENDERER_BACKEND_EXISTS = false; return; }\
			RENDERER.backend.settings = settings;\
			\
			if (_renderer_backend_create(client) != MINEC_CLIENT_SUCCESS) { RENDERER_BACKEND_EXISTS = false; RENDERER_CRASHING = true; }\
		}\
	}\
}\

RENDERER_BACKEND_SETTING_ACTION_FUNCTION(
	void renderer_backend_switch_device(struct minec_client* client, uint32_t device_index),
	RENDERER.backend.settings.backend_device_index != device_index,
	renderer_backend_apis[RENDERER.backend.settings.backend_index].switch_device(client, device_index)
)

RENDERER_BACKEND_SETTING_ACTION_FUNCTION(
	void renderer_backend_set_vsync(struct minec_client* client, bool vsync),
	RENDERER.backend.settings.vsync != vsync,
	renderer_backend_apis[RENDERER.backend.settings.backend_index].set_vsync(client, vsync)
)

RENDERER_BACKEND_SETTING_ACTION_FUNCTION(
	void renderer_backend_set_fps(struct minec_client* client, uint32_t fps),
	RENDERER.backend.settings.fps != fps,
	renderer_backend_apis[RENDERER.backend.settings.backend_index].set_fps(client, fps)
)

RENDERER_BACKEND_SETTING_ACTION_FUNCTION(
	void renderer_backend_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count),
	RENDERER.backend.settings.max_mipmap_level_count != max_mipmap_level_count,
	renderer_backend_apis[RENDERER.backend.settings.backend_index].set_max_mipmap_level_count(client, max_mipmap_level_count)
)

RENDERER_BACKEND_SETTING_ACTION_FUNCTION(
	void renderer_backend_reload_assets(struct minec_client* client),
	true,
	renderer_backend_apis[RENDERER.backend.settings.backend_index].reload_assets(client)
)


void renderer_backend_frame_begin(struct minec_client* client)
{
	RENDERER.backend.settings_frame_backup = RENDERER.backend.settings;

	if (renderer_backend_apis[RENDERER.backend.settings.backend_index].frame_begin(client) != MINEC_CLIENT_SUCCESS);
}

void renderer_backend_frame_menu(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.backend.settings.backend_index].frame_menu(client);
}

void renderer_backend_frame_end(struct minec_client* client)
{
	renderer_backend_apis[RENDERER.backend.settings.backend_index].frame_end(client);
}