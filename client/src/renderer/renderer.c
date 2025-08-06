#include <minec_client.h>

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

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
		RENDERER_ACCESS_REQUEST_STATE(requested_settings = RENDERER.public.request.settings;);

		if (RENDERER.settings.order_create_new_destroy_old != requested_settings.other.order_create_new_destroy_old)
		{
			RENDERER_ACCESS_INFO_STATE
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

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings)
{
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
		return MINEC_CLIENT_ERROR;
	}

	bool active = true;
	while (
		atomic_bool_load(&RENDERER.public.created) == false &&
		(active = atomic_bool_load(&RENDERER.public.active)) == true
		) time_sleep(1);

	if (active) return MINEC_CLIENT_SUCCESS;

	renderer_destroy(client);
	return MINEC_CLIENT_ERROR;
}

void renderer_destroy(struct minec_client* client)
{
	atomic_bool_store(&RENDERER.public.active, false);

	thread_join(RENDERER.thread_handle);

	mutex_destroy(&RENDERER.public.request.mutex);
	atomic_bool_deinit(&RENDERER.public.created);
	atomic_bool_deinit(&RENDERER.public.active);
	mutex_destroy(&RENDERER.public.info.mutex);
}

bool renderer_did_crash(struct minec_client* client)
{
	return !atomic_bool_load(&RENDERER.public.active);
}


bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	bool change = false;

	RENDERER_ACCESS_INFO_STATE
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

void renderer_set_order_create_new_destroy_old(struct minec_client* client, bool order)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.other.order_create_new_destroy_old = order;);
}

void renderer_switch_backend(struct minec_client* client, uint32_t backend_index)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.backend_index = backend_index;);
}

void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.backend_device_index = backend_device_index;);
}

void renderer_reload_resources(struct minec_client* client)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.reload_resources = true;);
}

void renderer_set_vsync(struct minec_client* client, bool vsync)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.vsync = vsync;);
}

void renderer_set_fps(struct minec_client* client, uint32_t fps)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.fps = fps;);
}

void renderer_set_gui_scale(struct minec_client* client, uint32_t scale)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.gui_scale = scale;);
}

void renderer_set_fov(struct minec_client* client, uint32_t fov)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.fov = fov;);
}

void renderer_set_render_distance(struct minec_client* client, uint32_t render_distance)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.frontend.render_distance = render_distance;);
}

void renderer_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	RENDERER_ACCESS_REQUEST_STATE(RENDERER.public.request.settings.backend.max_mipmap_level_count = max_mipmap_level_count;);
}

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY

uint32_t _renderer_create(struct minec_client* client, struct renderer_settings* settings)
{
	if ((client->renderer._renderer = malloc(sizeof(struct _renderer))) == NULL) return MINEC_CLIENT_ERROR;

	if (renderer_create(client, settings) != MINEC_CLIENT_SUCCESS)
	{
		free(client->renderer._renderer);
		return MINEC_CLIENT_ERROR;
	}

	return MINEC_CLIENT_SUCCESS;
}

void _renderer_destroy(struct minec_client* client)
{
	renderer_destroy(client);
	free(client->renderer._renderer);
}

EXPORT void renderer_get_api(struct renderer_client_api* api)
{
	api->_create = _renderer_create;
	api->_destroy = _renderer_destroy;
	api->_did_crash = renderer_did_crash;

	api->_get_info_state = renderer_get_info_state;

	api->_set_order_create_new_destroy_old = renderer_set_order_create_new_destroy_old;
	api->_switch_backend = renderer_switch_backend;
	api->_switch_backend_device = renderer_switch_backend_device;
	api->_set_vsync = renderer_set_vsync;
	api->_set_max_mipmap_level_count = renderer_set_max_mipmap_level_count;
	api->_reload_resources = renderer_reload_resources;
	api->_set_fps = renderer_set_fps;
	api->_set_fov = renderer_set_fov;
}

#endif 

#else

uint8_t* library_path = MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY_NAME;
uint8_t* library_copy_path = "temp_" MINEC_CLIENT_DYNAMIC_RENDERER_LIBRARY_NAME;

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;
	void (*renderer_get_api_func)(struct renderer_client_api* api);


	bool
		library_copied = false,
		library_loaded = false
	;

	if (result == MINEC_CLIENT_SUCCESS)
	{
		if (file_copy(library_path, library_copy_path) == 0) library_copied = true;
		else { result = MINEC_CLIENT_ERROR; minec_client_log_error(client, "[RENDERER] Failed to copy %s to %s", library_path, library_copy_path); minec_client_log_debug_l(client, "'file_copy(library_path, library_copy_path)' failed"); }
	}
	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((client->renderer.library_handle = dynamic_library_load(library_copy_path, true)) != NULL) library_loaded = true;
		else { result = MINEC_CLIENT_ERROR; minec_client_log_error(client, "[RENDERER] Failed to process dynamic library %s, may be corrupted", library_copy_path); minec_client_log_debug_l(client, "'dynamic_library_load(library_copy_path, true)' failed"); }
	}
	if (result == MINEC_CLIENT_SUCCESS)
	{
		if ((renderer_get_api_func = (void (*)(struct renderer_client_api* api))dynamic_library_get_function(client->renderer.library_handle, "renderer_get_api")) != NULL);
		else { result = MINEC_CLIENT_ERROR; minec_client_log_error(client, "[RENDERER] Failed to process dynamic library %s, may be old version", library_copy_path); minec_client_log_debug_l(client, "'dynamic_library_get_function(client->renderer.library_handle, \"renderer_get_api\")' failed"); }
	}

	if (result == MINEC_CLIENT_SUCCESS)
	{
		{
			client->renderer.client_renderer_api._application_window_get_dimensions = application_window_get_dimensions;

			client->renderer.client_renderer_api._log_info = minec_client_log_info;
			client->renderer.client_renderer_api._log_error = minec_client_log_error;

#ifdef MINEC_CLIENT_DEBUG_LOG
			client->renderer.client_renderer_api._log_debug = minec_client_log_debug;
			client->renderer.client_renderer_api._log_debug_l = _minec_client_log_debug_l;
#endif
		}

		renderer_get_api_func(&client->renderer.renderer_client_api);

		if (client->renderer.renderer_client_api._create(client, settings) == MINEC_CLIENT_SUCCESS);
		else { result = MINEC_CLIENT_ERROR; }
	}

	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (library_loaded) dynamic_library_unload(client->renderer.library_handle);
		if (library_copied) remove(library_copy_path);
	}

	return result;
}

void renderer_destroy(struct minec_client* client)
{
	client->renderer.renderer_client_api._destroy(client);

	dynamic_library_unload(client->renderer.library_handle);
	remove(library_copy_path);
}

bool renderer_did_crash(struct minec_client* client)
{
	return client->renderer.renderer_client_api._did_crash(client);
}


bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	return client->renderer.renderer_client_api._get_info_state(client, info_state);
}

uint32_t renderer_reload(struct minec_client* client)
{
	if (file_copy(library_copy_path, "minec_client_temp_file") != 0)
	{
		minec_client_log_error(client, "[RENDERER] Failed to copy '%s' to 'minec_client_temp_file'", library_copy_path);
		minec_client_log_debug_l(client, "'file_copy(client->renderer.library_copy_path, \"minec_client_temp_file\")' failed");
		return MINEC_CLIENT_SUCCESS;
	}

	struct renderer_settings settings = SETTINGS.video;

	renderer_destroy(client);

	if (renderer_create(client, &settings) != MINEC_CLIENT_SUCCESS)
	{
		minec_client_log_error(client, "[RENDERER] Failed to create new renderer, recreating old one");

		if (file_copy("minec_client_temp_file", library_path) != 0)
		{
			minec_client_log_error(client, "[RENDERER] Failed to copy 'minec_client_temp_file' to '%s'", "", library_path);
			minec_client_log_debug_l(client, "'file_copy(\"minec_client_temp_file\", library_path)' failed");
			return MINEC_CLIENT_ERROR;
		}

		if (renderer_create(client, &settings) != MINEC_CLIENT_SUCCESS)
		{
			minec_client_log_error(client, "[RENDERER] Failed to recreate old renderer");
			remove("minec_client_temp_file");
			return MINEC_CLIENT_ERROR;
		}

		remove("minec_client_temp_file");
		return MINEC_CLIENT_SUCCESS;
	}

	remove("minec_client_temp_file");
	return MINEC_CLIENT_SUCCESS;
}


void renderer_set_order_create_new_destroy_old(struct minec_client* client, bool order)
{
	client->renderer.renderer_client_api._set_order_create_new_destroy_old(client, order);
}

void renderer_switch_backend(struct minec_client* client, uint32_t backend_index)
{
	client->renderer.renderer_client_api._switch_backend(client, backend_index);
}
void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{
	client->renderer.renderer_client_api._switch_backend_device(client, backend_device_index);
}

void renderer_reload_resources(struct minec_client* client)
{
	client->renderer.renderer_client_api._reload_resources(client);
}

void renderer_set_vsync(struct minec_client* client, bool vsync)
{
	client->renderer.renderer_client_api._set_vsync(client, vsync);
}

void renderer_set_fps(struct minec_client* client, uint32_t fps)
{
	client->renderer.renderer_client_api._set_fps(client, fps);
}

void renderer_set_gui_scale(struct minec_client* client, uint32_t scale)
{
	client->renderer.renderer_client_api._set_gui_scale(client, scale);
}

void renderer_set_fov(struct minec_client* client, uint32_t fov)
{
	client->renderer.renderer_client_api._set_fov(client, fov);
}

void renderer_set_render_distance(struct minec_client* client, uint32_t render_distance)
{
	client->renderer.renderer_client_api._set_render_distance(client, render_distance);
}

void renderer_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	client->renderer.renderer_client_api._set_max_mipmap_level_count(client, max_mipmap_level_count);
}

#endif