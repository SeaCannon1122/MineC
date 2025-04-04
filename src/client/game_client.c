#include "game_client.h"

uint32_t game_client_run(struct game_client* game, uint8_t* resource_path) {

	application_create(&game->application_state);

	resources_create(game, resource_path);
	settings_load(game);

	gui_menus_create(game);
	renderer_create(game);

	simulator_start(game);
	networker_start(game);

#ifdef UNIX
	while (get_key_state(KEY_MOUSE_LEFT) & 0b1 != 0) sleep_for_ms(1);
#endif // UNIX

	while (application_handle_events(&game->application_state) == 0) {

		gui_menus_simulation_frame(game);

		if (gui_is_button_clicked(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.quit_game_button)) break;

		renderer_render(game);

		sleep_for_ms(16);
	}

	networker_stop(game);
	simulator_stop(game);

	renderer_destroy(game);
	gui_menus_destroy(game);

	settings_save(game);
	resources_destroy(game);

	application_destroy(&game->application_state);

	return 0;
}