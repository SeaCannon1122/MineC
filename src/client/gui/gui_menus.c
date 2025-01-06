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
	uint8_t ip_text[] = "IP-Address:Port";
	uint8_t invalid_syntax_text[] = "Invalid Syntax";
	uint8_t username_text[] = "Username";
	uint8_t password_text[] = "Password";
	uint8_t back_text[] = "Back";
	uint8_t join_text[] = "Join";

	struct game_char game_char_join_a_server_text[sizeof(join_a_server_text) - 1];
	struct game_char game_char_ip_text[sizeof(ip_text) - 1];
	struct game_char game_char_invalid_syntax_text[sizeof(invalid_syntax_text) - 1];
	struct game_char game_char_username_text[sizeof(username_text) - 1];
	struct game_char game_char_password_text[sizeof(password_text) - 1];
	struct game_char game_char_back_text[sizeof(back_text) - 1];
	struct game_char game_char_join_text[sizeof(join_text) - 1];

	for (uint32_t i = 0; i < sizeof(join_a_server_text) - 1; i++)
		game_char_join_a_server_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, join_a_server_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(ip_text) - 1; i++)
		game_char_ip_text[i] = (struct game_char){ {200, 200, 200, 255}, {0, 0, 0, 0}, ip_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(invalid_syntax_text) - 1; i++)
		game_char_invalid_syntax_text[i] = (struct game_char){ {200, 0, 0, 255}, {0, 0, 0, 0}, invalid_syntax_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(username_text) - 1; i++)
		game_char_username_text[i] = (struct game_char){ {200, 200, 200, 255}, {0, 0, 0, 0}, username_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(password_text) - 1; i++)
		game_char_password_text[i] = (struct game_char){ {200, 200, 200, 255}, {0, 0, 0, 0}, password_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(back_text) - 1; i++)
		game_char_back_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, back_text[i], PIXEL_CHAR_SHADOW_MASK };

	for (uint32_t i = 0; i < sizeof(join_text) - 1; i++)
		game_char_join_text[i] = (struct game_char){ {255, 255, 255, 255}, {0, 0, 0, 0}, join_text[i], PIXEL_CHAR_SHADOW_MASK };

	game->gui_menus_state.join_game.menu_handle = gui_scene_new(12, sizeof(join_a_server_text) + sizeof(ip_text) + + sizeof(invalid_syntax_text) + sizeof(username_text) + sizeof(password_text) + sizeof(back_text) + sizeof(join_text) - 7, 15 + 1 + 5 + GAME_MAX_USERNAME_LENGTH + GAME_MAX_PASSWORD_LENGTH);

	game->gui_menus_state.join_game.join_a_server_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(join_a_server_text) - 1);
	game->gui_menus_state.join_game.ip_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(ip_text) - 1);
	game->gui_menus_state.join_game.invalid_syntax_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(invalid_syntax_text) - 1);
	game->gui_menus_state.join_game.username_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(username_text) - 1);
	game->gui_menus_state.join_game.password_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(password_text) - 1);
	game->gui_menus_state.join_game.back_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(back_text) - 1);
	game->gui_menus_state.join_game.join_label = gui_add_label(game->gui_menus_state.join_game.menu_handle, sizeof(join_text) - 1);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_a_server_label, 0.5, 0, 0, 30, 0.5, 0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_label, 0.5, 0.5, -98, -31, 0.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.invalid_syntax_label, 0.5, 0.5, 98, -31, 1.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.username_label, 0.5, 0.5, -98, -1, 0.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.password_label, 0.5, 0.5, -98, 29, 0.0, 1.0, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_label, 0.5, 0.5, -51, 70, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_label, 0.5, 0.5, 51, 70, 0.5, 0.5, 0);

	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_a_server_label, game_char_join_a_server_text, sizeof(join_a_server_text) - 1, 4, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_label, game_char_ip_text, sizeof(ip_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.invalid_syntax_label, game_char_invalid_syntax_text, sizeof(invalid_syntax_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.username_label, game_char_username_text, sizeof(username_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.password_label, game_char_password_text, sizeof(password_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_label, game_char_back_text, sizeof(back_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_label, game_char_join_text, sizeof(join_text) - 1, 1, 4, 0.5);

	game->gui_menus_state.join_game.ip_textfield = gui_add_textfield(game->gui_menus_state.join_game.menu_handle, 15 + 1 + 5, &game->gui_menus_state.join_game.ip_textfield_ptr);
	game->gui_menus_state.join_game.username_textfield = gui_add_textfield(game->gui_menus_state.join_game.menu_handle, GAME_MAX_USERNAME_LENGTH, &game->gui_menus_state.join_game.username_textfield_ptr);
	game->gui_menus_state.join_game.password_textfield = gui_add_textfield(game->gui_menus_state.join_game.menu_handle, GAME_MAX_PASSWORD_LENGTH, &game->gui_menus_state.join_game.password_textfield_ptr);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_textfield, 0.5, 0.5, 0, -20, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.username_textfield, 0.5, 0.5, 0, 10, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.password_textfield, 0.5, 0.5, 0, 40, 0.5, 0.5, 0);

	uint32_t ip_default_text[] = { '1', '2', '7',  '.', '0',  '.',  '0',  '.',  '1',  ':',  '1',  '2',  '3',  '4',  '5' };
	uint32_t username_default_text[] = { 'S', 'e', 'a',  'C',  'a',  'n',  'n',  'o',  'n',  '1',  '1',  '2',  '2' };
	uint32_t password_default_text[] = { '1', '2', '3',  '4',  '5',  '6',  '7',  '8',  '9' };

	gui_set_textfield_text(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.ip_textfield, ip_default_text, sizeof(ip_default_text) / sizeof(ip_default_text[0]));
	gui_set_textfield_text(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.username_textfield, username_default_text, sizeof(username_default_text) / sizeof(username_default_text[0]));
	gui_set_textfield_text(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.password_textfield, password_default_text, sizeof(password_default_text) / sizeof(password_default_text[0]));

	game->gui_menus_state.join_game.back_button = gui_add_button(game->gui_menus_state.join_game.menu_handle, GUI_SIZE_SHORT);
	game->gui_menus_state.join_game.join_button = gui_add_button(game->gui_menus_state.join_game.menu_handle, GUI_SIZE_SHORT);

	gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 1);

	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_button, 0.5, 0.5, -51, 70, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 0.5, 0.5, 51, 70, 0.5, 0.5, 0);

	//server intermediate menu

	game->gui_menus_state.server_intermediate.menu_handle = gui_scene_new(4, sizeof(back_text) - 1 + 64 + MAX_CONNECTION_STATE_MESSAGE_LENGTH, 0);

	game->gui_menus_state.server_intermediate.back_label = gui_add_label(game->gui_menus_state.server_intermediate.menu_handle, sizeof(back_text) - 1);
	game->gui_menus_state.server_intermediate.status_label = gui_add_label(game->gui_menus_state.server_intermediate.menu_handle, 64);
	game->gui_menus_state.server_intermediate.message_label = gui_add_label(game->gui_menus_state.server_intermediate.menu_handle, MAX_CONNECTION_STATE_MESSAGE_LENGTH);

	gui_set_item_position(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.back_label, 0.5, 0.5, 0, 50, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.status_label, 0.5, 0.5, 0, -90, 0.5, 0.5, 0);
	gui_set_item_position(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.message_label, 0.5, 0.5, 0, -20, 0.5, 0.5, 0);

	gui_set_label(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.back_label, game_char_back_text, sizeof(back_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.status_label, game_char_back_text, sizeof(back_text) - 1, 1, 4, 0.5);
	gui_set_label(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.message_label, game_char_back_text, sizeof(back_text) - 1, 1, 4, 0.5);

	game->gui_menus_state.server_intermediate.back_button = gui_add_button(game->gui_menus_state.server_intermediate.menu_handle, GUI_SIZE_NORMAL);
	gui_set_item_position(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.back_button, 0.5, 0.5, 0, 50, 0.5, 0.5, 0);

	//

	game->gui_menus_state.active_menu = MENU_MAIN;

	return 0;
}

uint32_t gui_menus_destroy(struct game_client* game) {

	gui_scene_destroy(game->gui_menus_state.main.menu_handle);
	gui_scene_destroy(game->gui_menus_state.join_game.menu_handle);

	return 0;
}

uint32_t join_menu_check_syntax(struct game_client* game) {

	uint32_t* ip = game->gui_menus_state.join_game.ip_textfield_ptr;

	uint16_t ip_nums[4] = { 0, 0, 0 ,0 };
	uint32_t port = 0;

	uint32_t num_start = 0;

	for (uint32_t i = 0; i < 4; i++) {
		uint32_t num_i = 0;

		for (; (ip[num_start + num_i] >= '0' && ip[num_start + num_i] <= '9') && num_i < 3; num_i++)
			ip_nums[i] = ip_nums[i] * 10 + ip[num_start + num_i] - '0';

		if (num_i == 0 || (ip[num_start + num_i] != '.' && i < 3) || (ip[num_start + num_i] != ':' && i == 3) || ip_nums[i] > (uint16_t)UINT8_MAX) return 0;
		num_start += num_i + 1;
	}

	uint32_t num_i = 0;

	for (; (ip[num_start + num_i] >= '0' && ip[num_start + num_i] <= '9') && num_i < 5; num_i++)
		port = port * 10 + ip[num_start + num_i] - '0';

	if (num_i == 0 || ip[num_start + num_i] != 0 || port > (uint32_t)UINT16_MAX) return 0;

	return 1;
}

uint32_t gui_menus_simulation_frame(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: {

		gui_scene_simulate(game, game->gui_menus_state.main.menu_handle, game->settings_state.game_settings.gui_scale);

		if (gui_is_button_clicked(game->gui_menus_state.main.menu_handle, game->gui_menus_state.main.join_game_button)) game->gui_menus_state.active_menu = MENU_JOIN_GAME;

	} break;

	case MENU_JOIN_GAME: {

		gui_scene_simulate(game, game->gui_menus_state.join_game.menu_handle, game->settings_state.game_settings.gui_scale);

		if (join_menu_check_syntax(game)) gui_set_item_visibility(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.invalid_syntax_label, 0);
		else gui_set_item_visibility(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.invalid_syntax_label, 1);

		if (game_strlen(game->gui_menus_state.join_game.ip_textfield_ptr) && game_strlen(game->gui_menus_state.join_game.username_textfield_ptr) && game_strlen(game->gui_menus_state.join_game.password_textfield_ptr)) gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 0);
		else gui_enable_disable_button(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button, 1);

		if (gui_is_button_clicked(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.back_button)) game->gui_menus_state.active_menu = MENU_MAIN;
		if (gui_is_button_clicked(game->gui_menus_state.join_game.menu_handle, game->gui_menus_state.join_game.join_button)) {

			

			/*game->networker_state.ip_writing = 1;
			while (game->networker_state.ip_reading == 1);

			game->networker_state.ip_writing = 0;

			game->gui_menus_state.active_menu = MENU_SERVER_INTERMEDIATE;
			game->networker_state.request_flag_abort_connection = 0;
			game->networker_state.request_flag_connect = 1;*/
		}
		

	} break;

	case MENU_SERVER_INTERMEDIATE: {

		gui_scene_simulate(game, game->gui_menus_state.server_intermediate.menu_handle, game->settings_state.game_settings.gui_scale);

		if (gui_is_button_clicked(game->gui_menus_state.server_intermediate.menu_handle, game->gui_menus_state.server_intermediate.back_button)) {
			game->gui_menus_state.active_menu = MENU_JOIN_GAME;
			game->networker_state.request_flag_connect = 0;
			game->networker_state.request_flag_abort_connection = 1;
		}

	} break;

	}

	return 0;
}

uint32_t gui_menus_render(struct game_client* game) {

	switch (game->gui_menus_state.active_menu) {

	case MENU_MAIN: gui_scene_render(game, game->gui_menus_state.main.menu_handle, game->settings_state.game_settings.gui_scale); break;

	case MENU_JOIN_GAME: gui_scene_render(game, game->gui_menus_state.join_game.menu_handle, game->settings_state.game_settings.gui_scale); break;

	case MENU_SERVER_INTERMEDIATE: gui_scene_render(game, game->gui_menus_state.server_intermediate.menu_handle, game->settings_state.game_settings.gui_scale); break;

	}

	return 0;
}