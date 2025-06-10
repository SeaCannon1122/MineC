#include "renderer_internal.h"

uint32_t renderer_internal_create(
	struct minec_client* client,
	struct renderer_settings_state* request_settings_state,
	struct renderer_info_state** info_state,
	struct renderer_settings_state** settings_state
)
{
	printf("hello");

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
		*info_state = &client->renderer.renderer_internal_state->info_state;
		*settings_state = &client->renderer.renderer_internal_state->settings_state;
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

void renderer_internal_set_fps(struct minec_client* client, uint32_t fps)
{

}

void renderer_internal_set_vsync(struct minec_client* client, bool vsync)
{

}

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
#define RENDERER_INTERNAL_API EXPORT
#else
#define RENDERER_INTERNAL_API
#endif

RENDERER_INTERNAL_API void renderer_get_internal_interface(struct renderer_internal_interface* internal_interface)
{
	internal_interface->renderer_create = renderer_internal_create;
	internal_interface->renderer_destroy = renderer_internal_destroy;

	internal_interface->renderer_switch_backend = renderer_internal_switch_backend;
	internal_interface->renderer_switch_backend_device = renderer_internal_switch_backend_device;
	internal_interface->renderer_set_fps = renderer_internal_set_fps;
	internal_interface->renderer_set_vsync = renderer_internal_set_vsync;
}