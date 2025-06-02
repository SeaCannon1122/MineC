#include "minec_client.h"

uint32_t minec_client_run(struct minec_client* client, uint8_t* runtime_files_path)
{
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
	)) != MINEC_CLIENT_SUCCESS) goto _application_window_create_failed;
	minec_client_log(client, "[GLOBAL]", "Window created");

	settings_create(client);
	settings_load(client);

	resources_create(client);
	minec_client_log(client, "[GLOBAL]", "Resources created");

	if ((return_value = renderer_create(
		client, 
		&client->settings.video.graphics.backend_index,
		&client->settings.video.graphics.backend_count,
		&client->settings.video.graphics.backend_names,
		&client->settings.video.graphics.device_index,
		&client->settings.video.graphics.device_count,
		&client->settings.video.graphics.device_infos,
		&client->settings.video.graphics.fps
	)) != MINEC_CLIENT_SUCCESS) goto _renderer_create_failed;
	minec_client_log(client, "[GLOBAL]", "Renderer created");

	/*gui_menus_create(client);
	renderer_create(client);

	simulator_start(client);
	networker_start(client);*/

#ifdef UNIX
	while (get_key_state(KEY_MOUSE_LEFT) & 0b1 != 0) sleep_for_ms(1);
#endif // UNIX

	while (application_window_handle_events(client) == 0)
	{

		/*gui_menus_simulation_frame(client);

		if (gui_is_button_clicked(client->gui_menus_state.main.menu_handle, client->gui_menus_state.main.quit_game_button)) break;*/

		//renderer_render(client);

		sleep_for_ms(100);
	}

	/*networker_stop(client);
	simulator_stop(client);

	renderer_destroy(client);
	gui_menus_destroy(client);*/

	renderer_destroy(client);
	minec_client_log(client, "[GLOBAL]", "Renderer destroyed");

_renderer_create_failed:
	resources_destroy(client);
	minec_client_log(client, "[GLOBAL]", "Resources destroyed");

	settings_destroy(client);
	minec_client_log(client, "[GLOBAL]", "Window destroyed");

	application_window_destroy(client);

_application_window_create_failed:
	s_free(client->static_alloc, client->runtime_files_path);
	s_allocator_delete(client->static_alloc);
	s_allocator_delete(client->dynamic_alloc);

	return return_value;
}