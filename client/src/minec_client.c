#include "minec_client.h"


void minec_client_run(uint8_t* data_files_path)
{
	struct minec_client client_memory;
	struct minec_client* client = &client_memory;

	uint32_t return_value = MINEC_CLIENT_SUCCESS;

	client->static_alloc = s_allocator_new(4096);
	client->dynamic_alloc = s_allocator_new(4096);
	client->data_files_path = s_alloc_string(client->static_alloc, data_files_path);
	client->data_files_path_length = strlen(data_files_path);

	if ((return_value = application_window_create(
		client,
		100,
		100,
		700,
		500,
		"MineC"
	)) != MINEC_CLIENT_SUCCESS)
	{
		minec_client_log_info(client, "[GLOBAL] Failed to create Window");
		goto _application_window_create_failed;
	}
	minec_client_log_info(client, "[GLOBAL] Window created");

	settings_create(client);
	settings_load(client);

	struct renderer_settings settings;

	if ((return_value = renderer_create(client, &settings)) != MINEC_CLIENT_SUCCESS)
	{
		minec_client_log_error(client, "[GLOBAL] Failed to create Renderer ");
		goto _renderer_create_failed;
	}
	minec_client_log_info(client, "[GLOBAL] Renderer created");

	while (application_window_handle_events(client) == MINEC_CLIENT_SUCCESS)
	{

#ifdef MINEC_CLIENT_DYNAMIC_RENDERER
		if (client->window.input.keyboard[WINDOW_KEY_R] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
		{
			minec_client_log_info(client, "[GLOBAL] Reloading Renderer ...");
			if (renderer_reload(client) != MINEC_CLIENT_SUCCESS) minec_client_log_info(client, "[GLOBAL] Failed to reload Renderer");
		}
#endif

		sleep_for_ms(20);
	}

	renderer_destroy(client);
	minec_client_log_info(client, "[GLOBAL] Renderer destroyed");

_renderer_create_failed:

	settings_destroy(client);
	minec_client_log_info(client, "[GLOBAL] Window destroyed");

	application_window_destroy(client);

_application_window_create_failed:
	s_free(client->static_alloc, client->data_files_path);
	s_allocator_delete(client->static_alloc);
	s_allocator_delete(client->dynamic_alloc);
}