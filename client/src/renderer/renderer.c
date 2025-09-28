#include <minec_client.h>

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER_EXECUTABLE

void rendering_thread_function(struct minec_client* client)
{

	if (renderer_components_create(client) != MINEC_CLIENT_SUCCESS)
	{
		atomic_bool_store(&RENDERER.public.active, false);
		return;
	}

	atomic_bool_store(&RENDERER.public.created, true);

	while (atomic_bool_load(&RENDERER.public.active)) 
		if (renderer_frame(client) != MINEC_CLIENT_SUCCESS)
			atomic_bool_store(&RENDERER.public.active, false);

	renderer_components_destroy(client);
}

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings)
{
	RENDERER.settings = *settings;

	renderer_reset_action_state(client);
	RENDERER.public.state.info_changed = true;

	mutex_create(&RENDERER.public.action.mutex);
	mutex_create(&RENDERER.public.state.mutex);

	atomic_bool_init(&RENDERER.public.created, false);
	atomic_bool_init(&RENDERER.public.active, true);

	if ((RENDERER.thread_handle = thread_create(rendering_thread_function, client)) == NULL)
	{
		minec_client_log_debug_l(client, "thread_create failed");
		mutex_destroy(&RENDERER.public.action.mutex);
		atomic_bool_deinit(&RENDERER.public.created);
		atomic_bool_deinit(&RENDERER.public.active);
		mutex_destroy(&RENDERER.public.state.mutex);
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

	mutex_destroy(&RENDERER.public.action.mutex);
	atomic_bool_deinit(&RENDERER.public.created);
	atomic_bool_deinit(&RENDERER.public.active);
	mutex_destroy(&RENDERER.public.state.mutex);
}

bool renderer_did_crash(struct minec_client* client)
{
	return !atomic_bool_load(&RENDERER.public.active);
}


bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	bool change;

	RENDERER_ACCESS_PUBLIC_STATE
	(
		if ((change = RENDERER.public.state.info_changed) == true)
		{
			RENDERER.public.state.info_changed = false;

			*info_state = RENDERER.public.state.info;
		}
	);

	return change;
}

bool renderer_get_settings_state(struct minec_client* client, struct renderer_settings* settings_state)
{
	RENDERER_ACCESS_PUBLIC_STATE
	(
		*settings_state = RENDERER.public.state.settings;
	);

	return true;
}

void renderer_action(struct minec_client* client, struct renderer_action* action)
{
	RENDERER_ACCESS_ACTION_STATE(
		switch (action->type)
		{

		case _RENDERER_ACTION_SET_BACKEND_INDEX:
		{
			RENDERER.public.action.backend_index.value = action->parameters.backend_index;
			RENDERER.public.action.backend_index.set = true;
		} break;

		case _RENDERER_ACTION_SET_BACKEND_DEVICE_INDEX:
		{
			RENDERER.public.action.backend_device_index.value = action->parameters.backend_device_index;
			RENDERER.public.action.backend_device_index.set = true;
		} break;

		case _RENDERER_ACTION_SET_FPS:
		{
			RENDERER.public.action.fps.value = action->parameters.fps;
			RENDERER.public.action.fps.set = true;
		} break;

		case _RENDERER_ACTION_SET_VSYNC:
		{
			RENDERER.public.action.vsync.value = action->parameters.vsync;
			RENDERER.public.action.vsync.set = true;
		} break;

		case _RENDERER_ACTION_SET_MAX_MIPMAP_LEVEL_COUNT:
		{
			RENDERER.public.action.max_mipmap_level_count.value = action->parameters.max_mipmap_level_count;
			RENDERER.public.action.max_mipmap_level_count.set = true;
		} break;

		case _RENDERER_ACTION_SET_FOV:
		{
			RENDERER.public.action.fov.value = action->parameters.fov;
			RENDERER.public.action.fov.set = true;
		} break;

		case _RENDERER_ACTION_SET_RENDER_DISTANCE:
		{
			RENDERER.public.action.render_distance.value = action->parameters.render_distance;
			RENDERER.public.action.render_distance.set = true;
		} break;

		default:
			break;
		}
	);
}

void renderer_reset_action_state(struct minec_client* client)
{
	RENDERER.public.action.backend_device_index.set = false;
	RENDERER.public.action.backend_index.set = false;
	RENDERER.public.action.fov.set = false;
	RENDERER.public.action.fps.set = false;
	RENDERER.public.action.max_mipmap_level_count.set = false;
	RENDERER.public.action.render_distance.set = false;
	RENDERER.public.action.vsync.set = false;
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
	api->_get_settings_state = renderer_get_settings_state;
	api->_action = renderer_action;
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
			client->renderer.client_renderer_api._log_info = minec_client_log_info;
			client->renderer.client_renderer_api._log_error = minec_client_log_error;
#ifdef MINEC_CLIENT_DEBUG_LOG
			client->renderer.client_renderer_api._log_debug = minec_client_log_debug;
			client->renderer.client_renderer_api._log_debug_l = _minec_client_log_debug_l;
#endif
			client->renderer.client_renderer_api._asset_loader_get_asset = asset_loader_get_asset;
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

bool renderer_get_settings_state(struct minec_client* client, struct renderer_settings* settings_state)
{
	return client->renderer.renderer_client_api._get_settings_state(client, settings_state);
}

void renderer_action(struct minec_client* client, struct renderer_action* action)
{
	client->renderer.renderer_client_api._action(client, action);
}

uint32_t renderer_reload(struct minec_client* client)
{
	if (file_copy(library_copy_path, "minec_client_temp_file") != 0)
	{
		minec_client_log_error(client, "[RENDERER] Failed to copy '%s' to 'minec_client_temp_file'", library_copy_path);
		minec_client_log_debug_l(client, "'file_copy(client->renderer.library_copy_path, \"minec_client_temp_file\")' failed");
		return MINEC_CLIENT_SUCCESS;
	}

	struct renderer_settings settings = SETTINGS.video.renderer;

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

#endif