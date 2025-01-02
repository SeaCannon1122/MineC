#include "gui_menus.h"

#include "client/game_client.h"

uint32_t gui_menus_create(struct game_client* game) {


	//main menu
	uint8_t title_text[] = "This is not Minecraft";
	uint8_t join_game_text[] = "Join Game";
	uint8_t options_text[] = "Options";
	uint8_t quit_game_text[] = "Quit Game";

	struct game_char game_char_title_text[sizeof(title_text) - 1];
	struct game_char game_char_join_game_text[sizeof(join_game_text) - 1];
	struct game_char game_char_options_text[sizeof(options_text) - 1];
	struct game_char game_char_quit_game_text[sizeof(quit_game_text) - 1];

	for (uint32_t i = 0; i < sizeof(title_text) - 1; i++)
		game_char_title_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, title_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(join_game_text) - 1; i++)
		game_char_join_game_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, join_game_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(options_text) - 1; i++)
		game_char_options_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, options_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(quit_game_text) - 1; i++)
		game_char_quit_game_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, quit_game_text[i], PIXEL_CHAR_SHADOW_MASK };

	game->gui_menus_state.main.menu_handle = gui_scene_new(7, sizeof(title_text) + sizeof(join_game_text) + sizeof(options_text) + sizeof(quit_game_text) - 4, 0);

	game->gui_menus_state.main.title_label = gui_add_label(game->gui_menus_state.main.menu_handle, sizeof(title_text) - 1);
	game->gui_menus_state.main.join_game_label = gui_add_label(game->gui_menus_state.main.menu_handle, sizeof(join_game_text) - 1);
	game->gui_menus_state.main.options_label = gui_add_label(game->gui_menus_state.main.menu_handle, sizeof(options_text) - 1);
	game->gui_menus_state.main.quit_game_label = gui_add_label(game->gui_menus_state.main.menu_handle, sizeof(quit_game_text) - 1);

	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.title_label, 0.5, 0, 0, 30, 0.5, 0, 0);
	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.join_game_label, 0.5, 0.5, 0, 0, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.options_label, 0.5, 0.5, -51, 30, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.quit_game_label, 0.5, 0.5, 51, 30, 0.5, 0.5, 0);

	gui_set_label(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.title_label, game_char_title_text, sizeof(title_text) - 1, 4, 4, 0.5);
	gui_set_label(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.join_game_label, game_char_join_game_text, sizeof(join_game_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.options_label, game_char_options_text, sizeof(options_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.quit_game_label, game_char_quit_game_text, sizeof(quit_game_text) - 1, 1, 4, 0.5);

	game->gui_menus_state.main.join_game_button = gui_add_button(game->gui_menus_state.main.menu_handle, GUI_SIZE_NORMAL);
	game->gui_menus_state.main.options_button = gui_add_button(game->gui_menus_state.main.menu_handle, GUI_SIZE_SHORT);
	game->gui_menus_state.main.quit_game_button = gui_add_button(game->gui_menus_state.main.menu_handle, GUI_SIZE_SHORT);

	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.join_game_button, 0.5, 0.5, 0, 0, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.options_button, 0.5, 0.5, -51, 30, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.quit_game_button, 0.5, 0.5, 51, 30, 0.5, 0.5, 0);

	gui_enable_disable_button(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.options_button, 1);

	//join game menu

	uint8_t join_a_server_text[] = "Join a Server";
	uint8_t ip_text[] = "IP-Address";
	uint8_t port_text[] = "Port";
	uint8_t back_text[] = "Back";
	uint8_t join_text[] = "Join";

	struct game_char game_char_join_a_server_text[sizeof(join_a_server_text) - 1];
	struct game_char game_char_ip_text[sizeof(ip_text) - 1];
	struct game_char game_char_port_text[sizeof(port_text) - 1];
	struct game_char game_char_back_text[sizeof(back_text) - 1];
	struct game_char game_char_join_text[sizeof(join_text) - 1];

	for (uint32_t i = 0; i < sizeof(join_a_server_text) - 1; i++)
		game_char_join_a_server_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, join_a_server_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(ip_text) - 1; i++)
		game_char_ip_text[i] = (struct game_char){ {200, 200, 200, 255}, {0, 0, 0, 0}, ip_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(port_text) - 1; i++)
		game_char_port_text[i] = (struct game_char){ {200, 200, 200, 255}, {0, 0, 0, 0}, port_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(back_text) - 1; i++)
		game_char_back_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, back_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(join_text) - 1; i++)
		game_char_join_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, join_text[i], PIXEL_CHAR_SHADOW_MASK };

	game->gui_menus_state.join_game.menu_handle = gui_scene_new(9, sizeof(join_a_server_text) + sizeof(ip_text) + sizeof(port_text) + sizeof(back_text) + sizeof(join_text) - 5, 64 + 5);

	game->gui_menus_state.join_game.join_a_server_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(join_a_server_text) - 1);
	game->gui_menus_state.join_game.ip_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(ip_text) - 1);
	game->gui_menus_state.join_game.port_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(port_text) - 1);
	game->gui_menus_state.join_game.back_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(back_text) - 1);
	game->gui_menus_state.join_game.join_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(join_text) - 1);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_a_server_label, 0.5, 0, 0, 30, 0.5, 0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_label, 0.5, 0.5, -98, -31, 0.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.port_label, 0.5, 0.5, -98, 9, 0.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_label, 0.5, 0.5, -51, 50, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_label, 0.5, 0.5, 51, 50, 0.5, 0.5, 0);

	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_a_server_label, game_char_join_a_server_text, sizeof(join_a_server_text) - 1, 4, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_label, game_char_ip_text, sizeof(ip_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.port_label, game_char_port_text, sizeof(port_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_label, game_char_back_text, sizeof(back_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_label, game_char_join_text, sizeof(join_text) - 1, 1, 4, 0.5);

	game->gui_menus_state.join_game.ip_textfield = gui_add_textfield(game->gui_menus_state.join_game.menu_handle, 64, &game->gui_menus_state.join_game.ip_textfield_ptr);
	game->gui_menus_state.join_game.port_textfield = gui_add_textfield(game->gui_menus_state.join_game.menu_handle, 5, &game->gui_menus_state.join_game.port_textfield_ptr);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_textfield, 0.5, 0.5, 0, -20, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.port_textfield, 0.5, 0.5, 0, 20, 0.5, 0.5, 0);

	game->gui_menus_state.join_game.back_button = gui_add_button(game->gui_menus_state.join_game.menu_handle, GUI_SIZE_SHORT);
	game->gui_menus_state.join_game.join_button = gui_add_button(game->gui_menus_state.join_game.menu_handle, GUI_SIZE_SHORT);

	gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 1);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_button, 0.5, 0.5, -51, 50, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 0.5, 0.5, 51, 50, 0.5, 0.5, 0);

	game->gui_menus_state.active_menu = MENU_MAIN;

	return 0;
}

uint32_t gui_menus_destroy(struct game_client* game) {

	gui_scene_destroy(game->gui_menus_state.main.menu_handle);
	gui_scene_destroy(game->gui_menus_state.join_game.menu_handle);

	return 0;
}

uint32_t gui_menus_simulation_frame(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: {

		gui_scene_simulate(game, game->gui_menus_state.main.menu_handle, game->settings_state.game_settings.gui_scale);

		if (gui_is_button_clicked(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.join_game_button)) game->gui_menus_state.active_menu = MENU_JOIN_GAME;

	} break;

	case MENU_JOIN_GAME: {

		gui_scene_simulate(game, game->gui_menus_state.join_game.menu_handle, game->settings_state.game_settings.gui_scale);

		if (game_strlen(game->gui_menus_state.join_game.ip_textfield_ptr) && game_strlen(game->gui_menus_state.join_game.port_textfield_ptr)) gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 0);
		else gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 1);

		if (gui_is_button_clicked(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_button)) game->gui_menus_state.active_menu = MENU_MAIN;

		

	} break;

	}

}

uint32_t gui_menus_render(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: gui_scene_render(game, game->gui_menus_state.main.menu_handle, game->settings_state.game_settings.gui_scale); break;

	case MENU_JOIN_GAME: gui_scene_render(game, game->gui_menus_state.join_game.menu_handle, game->settings_state.game_settings.gui_scale); break;

	}

	return 0;
}