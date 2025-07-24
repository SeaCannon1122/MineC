#include <minec_client.h>

#ifndef MINEC_CLIENT_DYNAMIC_RENDERER
void renderer_get_api(struct renderer_api* api);
#endif

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
uint8_t* library_path = MINEC_CLIENT_SHARED_RENDERER_LIBRARY_NAME;
uint8_t* library_copy_path = "temp_" MINEC_CLIENT_SHARED_RENDERER_LIBRARY_NAME;
#endif

uint32_t renderer_create(struct minec_client* client, struct renderer_settings* settings)
{
	uint32_t result = MINEC_CLIENT_SUCCESS;
	void (*renderer_get_api_func)(struct renderer_api* api);

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER

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
		if ((renderer_get_api_func = (void (*)(struct renderer_api* api))dynamic_library_get_function(client->renderer.library_handle, "renderer_get_api")) != NULL);
		else { result = MINEC_CLIENT_ERROR; minec_client_log_error(client, "[RENDERER] Failed to process dynamic library %s, may be old version", library_copy_path); minec_client_log_debug_l(client, "'dynamic_library_get_function(client->renderer.library_handle, \"renderer_get_api\")' failed"); }
	}

#else
	renderer_get_api_func = renderer_get_api;
#endif

	if (result == MINEC_CLIENT_SUCCESS)
	{
		renderer_get_api_func(&client->renderer.api);

		if (client->renderer.api.create(client, settings) == MINEC_CLIENT_SUCCESS);
		else { result = MINEC_CLIENT_ERROR; }
	}

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	if (result != MINEC_CLIENT_SUCCESS)
	{
		if (library_loaded) dynamic_library_unload(client->renderer.library_handle);
		if (library_copied) remove(library_copy_path);
	}
#endif

	return result;
}

void renderer_destroy(struct minec_client* client)
{
	client->renderer.api.destroy(client);
#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
	dynamic_library_unload(client->renderer.library_handle);
	remove(library_copy_path);
#endif
}

bool renderer_did_crash(struct minec_client* client)
{
	return client->renderer.api.did_crash(client);
}


bool renderer_get_info_state(struct minec_client* client, struct renderer_info_state* info_state)
{
	return client->renderer.api.get_info_state(client, info_state);
}

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
uint32_t renderer_reload(struct minec_client* client)
{
	if (file_copy(library_copy_path, "minec_client_temp_file") != 0)
	{
		minec_client_log_error(client, "[RENDERER] Failed to copy %s to %s", library_copy_path, "minec_client_temp_file");
		minec_client_log_debug_l(client, "'file_copy(client->renderer.library_copy_path, \"minec_client_temp_file\")' failed");
		return MINEC_CLIENT_ERROR;
	}

	struct renderer_settings settings = client->renderer.settings_mirror;

	renderer_destroy(client);

	if (renderer_create(client, &settings) != MINEC_CLIENT_SUCCESS)
	{
		minec_client_log_error(client, "[RENDERER] Failed to create new renderer, recreating old one");

		if (file_copy("minec_client_temp_file", library_path) != 0)
		{
			minec_client_log_error(client, "[RENDERER] Failed to copy %s to %s, this application was not build for handeling this case: Crashing ...", "minec_client_temp_file", library_path);
			minec_client_log_debug_l(client, "'file_copy(\"minec_client_temp_file\", library_path)' failed");
			minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
		}

		if (renderer_create(client, &settings) != MINEC_CLIENT_SUCCESS)
		{
			minec_client_log_error(client, "[RENDERER] Failed to recreate old renderer, this application was not build for handeling this case: Crashing ...");
			minec_client_nuke_destroy_kill_crush_annihilate_process_exit(client);
		}

		remove("minec_client_temp_file");
		return MINEC_CLIENT_ERROR;
	}

	remove("minec_client_temp_file");
	return MINEC_CLIENT_SUCCESS;
}
#endif

void renderer_switch_backend(struct minec_client* client, uint32_t backend_index)
{
	client->renderer.api.switch_backend(client, backend_index);
}
void renderer_switch_backend_device(struct minec_client* client, uint32_t backend_device_index)
{
	client->renderer.api.switch_backend_device(client, backend_device_index);
}

void renderer_reload_resources(struct minec_client* client)
{
	client->renderer.api.reload_resources(client);
}

void renderer_set_vsync(struct minec_client* client, bool vsync)
{
	client->renderer.api.set_vsync(client, vsync);
}

void renderer_set_fps(struct minec_client* client, uint32_t fps)
{
	client->renderer.api.set_fps(client, fps);
}

void renderer_set_gui_scale(struct minec_client* client, uint32_t scale)
{
	client->renderer.api.set_gui_scale(client, scale);
}

void renderer_set_fov(struct minec_client* client, uint32_t fov)
{
	client->renderer.api.set_fov(client, fov);
}

void renderer_set_render_distance(struct minec_client* client, uint32_t render_distance)
{
	client->renderer.api.set_render_distance(client, render_distance);
}

void renderer_set_max_mipmap_level_count(struct minec_client* client, uint32_t max_mipmap_level_count)
{
	client->renderer.api.set_max_mipmap_level_count(client, max_mipmap_level_count);
}