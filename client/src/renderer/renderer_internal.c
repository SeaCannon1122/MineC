#include "renderer_internal.h"

void rendering_thread_function(struct minec_client* client)
{
	client->renderer.state->crashing = false;

	{
		uint32_t result = MINEC_CLIENT_SUCCESS;

		result = renderer_backend_initialize(client);

		if (result == MINEC_CLIENT_ERROR)
		{
			atomic_bool_store(&client->renderer.state->public.active, false);
			return;
		}
	}

	atomic_bool_store(&client->renderer.state->public.created, true);
	
	while (atomic_bool_load(&client->renderer.state->public.active))
	{

	}

}

void renderer_internal_destroy(struct minec_client* client);

uint32_t renderer_internal_create(struct minec_client* client, struct renderer_settings* settings)
{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	window_init_context(WINDOW.window_context_handle);
#endif

	if ((client->renderer.state = s_alloc(client->static_alloc, sizeof(struct renderer_internal_state))) == NULL)
	{
		minec_client_log_out_of_memory(client, "s_alloc(client->static_alloc, sizeof(struct renderer_internal_state))");
		return MINEC_CLIENT_ERROR;
	}

	RENDERER.backend_memory_index = 0;

	client->renderer.state->public.info.changed = true;
	mutex_create(&client->renderer.state->public.info.mutex);

	atomic_bool_init(&client->renderer.state->public.created, false);
	atomic_bool_init(&client->renderer.state->public.active, true);

	mutex_create(&client->renderer.state->public.settings.mutex);
	if ((client->renderer.state->thread_handle = thread_create(rendering_thread_function, client)) == NULL)
	{
		minec_client_log_debug_error(client, "thread_create failed");
		mutex_destroy(&client->renderer.state->public.settings.mutex);
		atomic_bool_deinit(&client->renderer.state->public.created);
		atomic_bool_deinit(&client->renderer.state->public.active);
		mutex_destroy(&client->renderer.state->public.info.mutex);
		s_free(client->static_alloc, client->renderer.state);
		return MINEC_CLIENT_ERROR;
	}

	bool active = true;
	while (
		atomic_bool_load(&client->renderer.state->public.created) == false && 
		(active = atomic_bool_load(&client->renderer.state->public.active)) == true
	) time_sleep(1);

	if (active) return MINEC_CLIENT_SUCCESS;
	
	renderer_internal_destroy(client);
	return MINEC_CLIENT_ERROR;
}

void renderer_internal_destroy(struct minec_client* client)
{
	atomic_bool_store(&client->renderer.state->public.active, false);

	thread_join(client->renderer.state->thread_handle);

	mutex_destroy(&client->renderer.state->public.settings.mutex);
	atomic_bool_deinit(&client->renderer.state->public.created);
	atomic_bool_deinit(&client->renderer.state->public.active);
	mutex_destroy(&client->renderer.state->public.info.mutex);
	s_free(client->static_alloc, client->renderer.state);
}

bool renderer_internal_did_crash(struct minec_client* client)
{
	return atomic_bool_load(&client->renderer.state->public.active);
}


bool renderer_internal_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	bool change = false;
	mutex_lock(&client->renderer.state->public.info.mutex);

	if (client->renderer.state->public.info.changed)
	{
		change = true;
		client->renderer.state->public.info.changed = false;

		*info_state = client->renderer.state->public.info.state;
	}

	mutex_unlock(&client->renderer.state->public.info.mutex);

	return change;
}

void renderer_internal_switch_backend(struct minec_client* client, uint32_t backend_index)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.backend_index = backend_index;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.backend_device_index = backend_device_index;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_reload_resources(struct minec_client* client)
{

}

void renderer_internal_set_vsync(struct minec_client* client, bool vsync)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.vsync = vsync;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_set_fps(struct minec_client* client, uint32_t fps)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.fps = fps;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_set_gui_scale(struct minec_client* client, uint32_t scale)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.gui_scale = scale;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_set_fov(struct minec_client* client, uint32_t fov)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.fov = fov;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_set_render_distance(struct minec_client* client, uint32_t render_distance)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.render_distance = render_distance;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
}

void renderer_internal_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	mutex_lock(&client->renderer.state->public.settings.mutex);
	client->renderer.state->public.settings.max_mipmap_level_count = max_mipmap_level_count;
	mutex_unlock(&client->renderer.state->public.settings.mutex);
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
	api->did_crash = renderer_internal_did_crash;

	api->get_info_state = renderer_internal_get_info_state;

	api->switch_backend = renderer_internal_switch_backend;
	api->switch_backend_device = renderer_internal_switch_backend_device;
	api->set_vsync = renderer_internal_set_vsync;
	api->set_max_mipmap_level_count = renderer_internal_set_max_mipmap_level_count;
	api->reload_resources = renderer_internal_reload_resources;
	api->set_fps = renderer_internal_set_fps;
	api->set_fov = renderer_internal_set_fov;
}