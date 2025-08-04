 #include "renderer_internal.h"

void rendering_thread_function(struct minec_client* client)
{
	RENDERER.crashing = false;
	RENDERER.settings = RENDERER.public.request.settings.other;

	{
		bool
			frontend_created = false,
			backend_created = false
		;

		uint32_t result = MINEC_CLIENT_SUCCESS;
		if (result == MINEC_CLIENT_SUCCESS)
		{
			if ((result = renderer_frontend_create(client)) != MINEC_CLIENT_SUCCESS)
				minec_client_log_error(client, "[RENDERER] failed to create frontend");
			else frontend_created = true;
		}
		if (result == MINEC_CLIENT_SUCCESS)
		{
			if ((result = renderer_backend_create(client)) != MINEC_CLIENT_SUCCESS)
				minec_client_log_error(client, "[RENDERER] failed to create backend");
			else backend_created = true;
		}
		
		if (result != MINEC_CLIENT_SUCCESS)
		{
			if (backend_created) renderer_backend_destroy(client);
			if (frontend_created) renderer_frontend_destroy(client);

			atomic_bool_store(&RENDERER.public.active, false);
			return;
		}
	}

	atomic_bool_store(&RENDERER.public.created, true);
	
	while (atomic_bool_load(&RENDERER.public.active))
	{
		struct renderer_settings requested_settings;
		ACCESS_REQUEST_STATE(requested_settings = RENDERER.public.request.settings;);

		if (RENDERER.settings.order_create_new_destroy_old != requested_settings.other.order_create_new_destroy_old)
		{
			ACCESS_INFO_STATE
			(
				RENDERER.public.info.state.settings.other.order_create_new_destroy_old = requested_settings.other.order_create_new_destroy_old;
				RENDERER.public.info.changed = true;
			);
			RENDERER.settings.order_create_new_destroy_old = requested_settings.other.order_create_new_destroy_old;
		}

		renderer_frontend_frame(client);
		if (RENDERER.crashing) break;
		renderer_backend_frame(client);
		if (RENDERER.crashing) break;
	}

	renderer_backend_destroy(client);
	renderer_frontend_destroy(client);

	atomic_bool_store(&RENDERER.public.active, false);
}

void renderer_internal_destroy(struct minec_client* client);

uint32_t renderer_internal_create(struct minec_client* client, struct renderer_settings* settings)
{
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	window_init_context(WINDOW.window_context_handle);
#endif

	if ((client->renderer.state = malloc(sizeof(struct renderer_internal_state))) == NULL)
	{
		minec_client_log_out_of_memory(client, "s_alloc(client->static_alloc, sizeof(struct renderer_internal_state))");
		return MINEC_CLIENT_ERROR;
	}

	RENDERER.public.request.settings = *settings;
	RENDERER.public.request.reload_resources = false;
	RENDERER.public.request.reload_resources_detected = false;

	RENDERER.public.info.changed = true;

	mutex_create(&RENDERER.public.request.mutex);
	mutex_create(&RENDERER.public.info.mutex);

	atomic_bool_init(&RENDERER.public.created, false);
	atomic_bool_init(&RENDERER.public.active, true);

	
	if ((RENDERER.thread_handle = thread_create(rendering_thread_function, client)) == NULL)
	{
		minec_client_log_debug_l(client, "thread_create failed");
		mutex_destroy(&RENDERER.public.request.mutex);
		atomic_bool_deinit(&RENDERER.public.created);
		atomic_bool_deinit(&RENDERER.public.active);
		mutex_destroy(&RENDERER.public.info.mutex);
		free(client->renderer.state);
		return MINEC_CLIENT_ERROR;
	}

	bool active = true;
	while (
		atomic_bool_load(&RENDERER.public.created) == false && 
		(active = atomic_bool_load(&RENDERER.public.active)) == true
	) time_sleep(1);

	if (active) return MINEC_CLIENT_SUCCESS;
	
	renderer_internal_destroy(client);
	return MINEC_CLIENT_ERROR;
}

void renderer_internal_destroy(struct minec_client* client)
{
	atomic_bool_store(&RENDERER.public.active, false);

	thread_join(RENDERER.thread_handle);

	mutex_destroy(&RENDERER.public.request.mutex);
	atomic_bool_deinit(&RENDERER.public.created);
	atomic_bool_deinit(&RENDERER.public.active);
	mutex_destroy(&RENDERER.public.info.mutex);
	free(client->renderer.state);
}

bool renderer_internal_did_crash(struct minec_client* client)
{
	return !atomic_bool_load(&RENDERER.public.active);
}


bool renderer_internal_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	bool change = false;
	
	ACCESS_INFO_STATE
	(
		if (RENDERER.public.info.changed)
		{
			change = true;
			RENDERER.public.info.changed = false;

			*info_state = RENDERER.public.info.state;
		}
	);

	return change;
}

void renderer_internal_set_order_create_new_destroy_old(struct minec_client* client, bool order)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.other.order_create_new_destroy_old = order;);
}

void renderer_internal_switch_backend(struct minec_client* client, uint32_t backend_index)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.backend_index = backend_index;);
}

void renderer_internal_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.backend_device_index = backend_device_index;);
}

void renderer_internal_reload_resources(struct minec_client* client)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.reload_resources = true;);
}

void renderer_internal_set_vsync(struct minec_client* client, bool vsync)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.vsync = vsync;);
}

void renderer_internal_set_fps(struct minec_client* client, uint32_t fps)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.fps = fps;);
}

void renderer_internal_set_gui_scale(struct minec_client* client, uint32_t scale)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.gui_scale = scale;);
}

void renderer_internal_set_fov(struct minec_client* client, uint32_t fov)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.fov = fov;);
}

void renderer_internal_set_render_distance(struct minec_client* client, uint32_t render_distance)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.render_distance = render_distance;);
}

void renderer_internal_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.max_mipmap_level_count = max_mipmap_level_count;);
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

	api->set_order_create_new_destroy_old = renderer_internal_set_order_create_new_destroy_old;
	api->switch_backend = renderer_internal_switch_backend;
	api->switch_backend_device = renderer_internal_switch_backend_device;
	api->set_vsync = renderer_internal_set_vsync;
	api->set_max_mipmap_level_count = renderer_internal_set_max_mipmap_level_count;
	api->reload_resources = renderer_internal_reload_resources;
	api->set_fps = renderer_internal_set_fps;
	api->set_fov = renderer_internal_set_fov;
}