#include "minec_client.h"


void minec_client_run(uint8_t* runtime_files_path)
{
	struct minec_client client_memory;
	struct minec_client* client = &client_memory;

	uint32_t return_value = MINEC_CLIENT_SUCCESS;

	client->static_alloc = s_allocator_new(4096);
	client->dynamic_alloc = s_allocator_new(4096);
	client->runtime_files_path = s_alloc_string(client->static_alloc, runtime_files_path);
	client->runtime_files_path_length = strlen(runtime_files_path);

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

	resources_create(client);
	minec_client_log_info(client, "[GLOBAL] Resources created");

	struct renderer_settings_state request_renderer_settings;
	struct renderer_info_state* renderer_infos;
	struct renderer_settings_state* renderer_settings;

	if ((return_value = renderer_create(
		client,
		&request_renderer_settings,
		&renderer_infos,
		&renderer_settings
	)) != MINEC_CLIENT_SUCCESS)
	{
		minec_client_log_error(client, "[GLOBAL] Failed to create Renderer ");
		goto _renderer_create_failed;
	}
	minec_client_log_info(client, "[GLOBAL] Renderer created");

	/*gui_menus_create(client);

	simulator_start(client);
	networker_start(client);*/

	while (application_window_handle_events(client) == 0)
	{

		/*gui_menus_simulation_frame(client);

		if (gui_is_button_clicked(client->gui_menus_state.main.menu_handle, client->gui_menus_state.main.quit_game_button)) break;*/

		//renderer_render(client);

		if (client->window.input.keyboard[WINDOW_KEY_R] == (KEY_DOWN_MASK | KEY_CHANGE_MASK))
		{
			minec_client_log_info(client, "[GLOBAL] Reloading Renderer ...");

			if (renderer_reload(
				client,
				&renderer_infos,
				&renderer_settings
			) != MINEC_CLIENT_SUCCESS) minec_client_log_info(client, "[GLOBAL] Failed to reload Renderer");
		}

		sleep_for_ms(20);
	}

	/*networker_stop(client);
	simulator_stop(client);

	gui_menus_destroy(client);*/

	renderer_destroy(client);
	minec_client_log_info(client, "[GLOBAL] Renderer destroyed");

_renderer_create_failed:
	resources_destroy(client);
	minec_client_log_info(client, "[GLOBAL] Resources destroyed");

	settings_destroy(client);
	minec_client_log_info(client, "[GLOBAL] Window destroyed");

	application_window_destroy(client);

_application_window_create_failed:
	s_free(client->static_alloc, client->runtime_files_path);
	s_allocator_delete(client->static_alloc);
	s_allocator_delete(client->dynamic_alloc);
}