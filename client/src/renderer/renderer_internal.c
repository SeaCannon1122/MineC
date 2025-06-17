#include "renderer_internal.h"

uint32_t renderer_internal_create(struct minec_client* client, struct renderer_settings* settings)
{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	window_init_context(client->window.window_context_handle);
#endif

	uint32_t result = MINEC_CLIENT_SUCCESS;
	bool
		internal_state_memory_allocated = false
	;
	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((client->renderer.renderer_internal_state = s_alloc(client->static_alloc, sizeof(struct renderer_internal_state))) != NULL) internal_state_memory_allocated = true;
		else { result = MINEC_CLIENT_ERROR; minec_client_log_out_of_memory(client, "s_alloc(client->static_alloc, sizeof(struct renderer_internal_state))"); }
	}
	
	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (internal_state_memory_allocated) s_free(client->static_alloc, client->renderer.renderer_internal_state);
	}
	else
	{

	}

	return result;
}

void renderer_internal_destroy(struct minec_client* client)
{
	s_free(client->static_alloc, client->renderer.renderer_internal_state);
}

void renderer_internal_switch_backend(struct minec_client* client, uint32_t backend_index)
{

}

void renderer_internal_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{

}

void renderer_internal_reload_resources(struct minec_client* client)
{

}

void renderer_internal_set_vsync(struct minec_client* client, bool vsync)
{

}

void renderer_internal_set_fps(struct minec_client* client, uint32_t fps)
{

}

void renderer_internal_set_fov(struct minec_client* client, uint32_t fov)
{

}

void renderer_internal_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{

}

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
#define RENDERER_INTERNAL_API EXPORT
#else
#define RENDERER_INTERNAL_API
#endif

RENDERER_INTERNAL_API void renderer_get_api(struct renderer_api* api)
{
	api->create = renderer_internal_create;
	api->destroy = renderer_internal_destroy;
	api->switch_backend = renderer_internal_switch_backend;
	api->switch_backend_device = renderer_internal_switch_backend_device;
	api->set_vsync = renderer_internal_set_vsync;
	api->set_max_mipmap_level_count = renderer_internal_set_max_mipmap_level_count;
	api->reload_resources = renderer_internal_reload_resources;
	api->set_fps = renderer_internal_set_fps;
	api->set_fov = renderer_internal_set_fov;
}