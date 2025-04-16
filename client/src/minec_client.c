#include "minec_client.h"

uint32_t minec_client_run(struct minec_client* client, uint8_t* resource_path) {

	application_create(&client->application_state);

	resources_create(client, resource_path);
	settings_load(client);

	gui_menus_create(client);
	renderer_create(client);

	simulator_start(client);
	networker_start(client);

#ifdef UNIX
	while (get_key_state(KEY_MOUSE_LEFT) & 0b1 != 0) sleep_for_ms(1);
#endif // UNIX

	while (application_handle_events(&client->application_state) == 0) {

		gui_menus_simulation_frame(client);

		if (gui_is_button_clicked(client->gui_menus_state.main.menu_handle, client->gui_menus_state.main.quit_game_button)) break;

		renderer_render(client);

		sleep_for_ms(16);
	}

	networker_stop(client);
	simulator_stop(client);

	renderer_destroy(client);
	gui_menus_destroy(client);

	settings_save(client);
	resources_destroy(client);

	application_destroy(&client->application_state);

	return 0;
}