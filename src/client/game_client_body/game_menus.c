#include "game_menus.h"

#include <stdlib.h>

#include "general/platformlib/platform.h"
#include "general/keyvalue.h"
#include "general/utils.h"
#include "client/gui/char_font.h"
#include "client/gui/menu.h"
#include "game_client.h"


void init_game_menus(struct game_client* game) {

	game->game_menus.active_menu = MAIN_MENU;



	struct argb_image* background = get_value_from_key(game->resource_manager, "menu_background").ptr;

	struct argb_image* light_texture = get_value_from_key(game->resource_manager, "menu_button_enabled").ptr;
	struct argb_image* dark_texture = get_value_from_key(game->resource_manager, "menu_button_disabled").ptr;

	struct char_font* default_font = get_value_from_key(game->resource_manager, "default_pixelfont").ptr;




	game->game_menus.main_menu.menu.menu_items_count = 0;

	game->game_menus.main_menu.join_game_button_enabled = true;
	game->game_menus.main_menu.join_game_button_state = false;
	game->game_menus.main_menu.options_button_enabled = true;
	game->game_menus.main_menu.options_button_state = false;
	game->game_menus.main_menu.quit_game_button_enabled = true;
	game->game_menus.main_menu.quit_game_button_state = false;

	convert_string_to_gui_string_in_buffer(default_font, "This is not Minecraft!!!", 4, 0xffffffff, game->game_menus.main_menu.this_is_not_minecraft_text, 25 );
	convert_string_to_gui_string_in_buffer(default_font, "Join Game", 1, 0xffffffff, game->game_menus.main_menu.join_game_text, 10 );
	convert_string_to_gui_string_in_buffer(default_font, "Options...", 1, 0xffffffff, game->game_menus.main_menu.options_text, 11 );
	convert_string_to_gui_string_in_buffer(default_font, "Quit Game", 1, 0xffffffff, game->game_menus.main_menu.quit_game_text, 10);

	add_menu_image(&game->game_menus.main_menu.menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 7);

	add_menu_label(&game->game_menus.main_menu.menu, 10, 0, 60, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.main_menu.this_is_not_minecraft_text, ALIGNMENT_MIDDLE);
	add_menu_label(&game->game_menus.main_menu.menu, 10, 0, 0, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.main_menu.join_game_text, ALIGNMENT_MIDDLE);
	add_menu_label(&game->game_menus.main_menu.menu, 10, -49, 60, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.main_menu.options_text, ALIGNMENT_MIDDLE);
	add_menu_label(&game->game_menus.main_menu.menu, 10, 49, 60, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.main_menu.quit_game_text, ALIGNMENT_MIDDLE);

	add_menu_button(&game->game_menus.main_menu.menu, 1, &game->game_menus.main_menu.join_game_button_state, -100, -10, 100, 10, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &game->game_menus.main_menu.join_game_button_enabled);
	add_menu_button(&game->game_menus.main_menu.menu, 1, &game->game_menus.main_menu.options_button_state, -100, 50, -2, 70, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &game->game_menus.main_menu.options_button_enabled);
	add_menu_button(&game->game_menus.main_menu.menu, 1, &game->game_menus.main_menu.quit_game_button_state, 2, 50, 100, 70, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &game->game_menus.main_menu.quit_game_button_enabled);





	game->game_menus.options_menu.menu.menu_items_count = 0;

	game->game_menus.options_menu.done_button_enabled = true;
	game->game_menus.options_menu.done_button_state = false;
	game->game_menus.options_menu.fov_slider_state = ((float)game->settings.fov - (float)FOV_MIN) / ((float)FOV_MAX - (float)FOV_MIN);
	game->game_menus.options_menu.render_distance_slider_state = ((float)game->settings.render_distance - (float)RENDER_DISTANCE_MIN) / ((float)RENDER_DISTANCE_MAX - (float)RENDER_DISTANCE_MIN);
	game->game_menus.options_menu.gui_scale_button_enabled = true;
	game->game_menus.options_menu.gui_scale_button_state = false;
	

	convert_string_to_gui_string_in_buffer(default_font, "Options", 2, 0xffffffff, game->game_menus.options_menu.options_text, 8);
	convert_string_to_gui_string_in_buffer(default_font, "FOV:    ", 1, 0xffffffff, game->game_menus.options_menu.fov_text, 9);

	game->game_menus.options_menu.fov_text[5].value = (game->settings.fov < 100 ? '\x1f' : digit_to_char(game->settings.fov / 100));
	game->game_menus.options_menu.fov_text[6].value = (game->settings.fov < 10 ? '\x1f' : digit_to_char((game->settings.fov / 10) % 10));
	game->game_menus.options_menu.fov_text[7].value = digit_to_char(game->settings.fov % 10);

	convert_string_to_gui_string_in_buffer(default_font, "Render Distance:    Chunks", 1, 0xffffffff, game->game_menus.options_menu.render_distance_text, 27);

	game->game_menus.options_menu.render_distance_text[17].value = (game->settings.render_distance < 10 ? '\x1f' : digit_to_char(game->settings.render_distance / 10));
	game->game_menus.options_menu.render_distance_text[18].value = digit_to_char(game->settings.render_distance % 10);

	convert_string_to_gui_string_in_buffer(default_font, "GUI Scale:     ", 1, 0xffffffff, game->game_menus.options_menu.gui_scale_text, 16);

	if (game->settings.gui_scale > 0) {
		game->game_menus.options_menu.gui_scale_text[11].value = digit_to_char(game->settings.gui_scale);
		game->game_menus.options_menu.gui_scale_text[12].value = '\x1f';
		game->game_menus.options_menu.gui_scale_text[13].value = '\x1f';
		game->game_menus.options_menu.gui_scale_text[14].value = '\x1f';
	}
	else {
		game->game_menus.options_menu.gui_scale_text[11].value = 'A';
		game->game_menus.options_menu.gui_scale_text[12].value = 'u';
		game->game_menus.options_menu.gui_scale_text[13].value = 't';
		game->game_menus.options_menu.gui_scale_text[14].value = 'o';
	}

	convert_string_to_gui_string_in_buffer(default_font, "Done", 1, 0xffffffff, game->game_menus.options_menu.done_text, 5);

	add_menu_image(&game->game_menus.options_menu.menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 7);

	add_menu_label(&game->game_menus.options_menu.menu, 2, -79, 60, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.options_menu.fov_text, ALIGNMENT_MIDDLE); //fov
	add_menu_label(&game->game_menus.options_menu.menu, 2, 79, 60, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.options_menu.render_distance_text, ALIGNMENT_MIDDLE); //render distance
	add_menu_label(&game->game_menus.options_menu.menu, 2, -79, 85, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.options_menu.gui_scale_text, ALIGNMENT_MIDDLE); //gui_scale

	add_menu_label(&game->game_menus.options_menu.menu, 0, 0, 25, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.options_menu.options_text, ALIGNMENT_MIDDLE);


	add_menu_slider(&game->game_menus.options_menu.menu, 1, &game->game_menus.options_menu.fov_slider_state, -154, 50, -4, 70, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, dark_texture, light_texture, 10);
	add_menu_slider(&game->game_menus.options_menu.menu, 1, &game->game_menus.options_menu.render_distance_slider_state, 4, 50, 154, 70, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, dark_texture, light_texture, 10);

	add_menu_button(&game->game_menus.options_menu.menu, 1, &game->game_menus.options_menu.gui_scale_button_state, -154, 75, -4, 95, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, light_texture, dark_texture, &game->game_menus.options_menu.gui_scale_button_enabled);

	add_menu_label(&game->game_menus.options_menu.menu, 10, 0, -15, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, game->game_menus.options_menu.done_text, ALIGNMENT_MIDDLE);
	add_menu_button(&game->game_menus.options_menu.menu, 2, &game->game_menus.options_menu.done_button_state, -100, -25, 100, -5, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, light_texture, dark_texture, &game->game_menus.options_menu.done_button_enabled);




	game->game_menus.join_game_menu.menu.menu_items_count = 0;

	game->game_menus.join_game_menu.join_game_button_enabled = false;
	game->game_menus.join_game_menu.join_game_button_state = false;
	game->game_menus.join_game_menu.back_button_state = false;
	game->game_menus.join_game_menu.back_button_enabled = true;
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.ip_address_buffer); i++) game->game_menus.join_game_menu.ip_address_buffer[i] = '\0';
	game->game_menus.join_game_menu.ip_address_buffer_link = -1;
	game->game_menus.join_game_menu.ip_address_box_selected = false;
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.port_buffer); i++) game->game_menus.join_game_menu.port_buffer[i] = '\0';
	game->game_menus.join_game_menu.port_buffer_link = -1;
	game->game_menus.join_game_menu.port_box_selected = false;
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.username_buffer); i++) game->game_menus.join_game_menu.username_buffer[i] = '\0';
	game->game_menus.join_game_menu.username_buffer_link = -1;
	game->game_menus.join_game_menu.username_box_selected = false;
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.password_buffer); i++) game->game_menus.join_game_menu.password_buffer[i] = '\0';
	game->game_menus.join_game_menu.password_buffer_link = -1;
	game->game_menus.join_game_menu.password_box_selected = false;

	convert_string_to_gui_string_in_buffer(default_font, "Back", 1, 0xffffffff, game->game_menus.join_game_menu.back_text, 5);
	convert_string_to_gui_string_in_buffer(default_font, "Join a multiplayer game", 2, 0xffffffff, game->game_menus.join_game_menu.join_a_multiplayer_game_text, 24);

	convert_string_to_gui_string_in_buffer(default_font, "IP-address:", 1, 0xffa0a0a0, game->game_menus.join_game_menu.ip_address_text, 12);
	convert_string_to_gui_string_in_buffer(default_font, "Port:", 1, 0xffa0a0a0, game->game_menus.join_game_menu.port_text, 6);
	convert_string_to_gui_string_in_buffer(default_font, "Username:", 1, 0xffa0a0a0, game->game_menus.join_game_menu.username_text, 10);
	convert_string_to_gui_string_in_buffer(default_font, "Password:", 1, 0xffa0a0a0, game->game_menus.join_game_menu.password_text, 10);

	convert_string_to_gui_string_in_buffer(default_font, "Join Game", 1, 0xffffffff, game->game_menus.join_game_menu.join_game_text, 10);

	add_menu_image(&game->game_menus.join_game_menu.menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 7);

	add_menu_label(&game->game_menus.join_game_menu.menu, 0, 0, 25, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.join_game_menu.join_a_multiplayer_game_text, ALIGNMENT_MIDDLE);

	add_menu_label(&game->game_menus.join_game_menu.menu, 11, 0, -15, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, game->game_menus.join_game_menu.back_text, ALIGNMENT_MIDDLE);
	add_menu_button(&game->game_menus.join_game_menu.menu, 10, &game->game_menus.join_game_menu.back_button_state, -100, -25, 100, -5, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, light_texture, dark_texture, &game->game_menus.join_game_menu.back_button_enabled);

	add_menu_label(&game->game_menus.join_game_menu.menu, 2, -123, -48, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.join_game_menu.ip_address_text, ALIGNMENT_LEFT);
	add_menu_text_field(&game->game_menus.join_game_menu.menu, 2, game->game_menus.join_game_menu.ip_address_buffer,  -125, 45, -30, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, ALIGNMENT_LEFT, &game->game_menus.join_game_menu.ip_address_box_selected, default_font);

	add_menu_label(&game->game_menus.join_game_menu.menu, 2, 57, -48, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.join_game_menu.port_text, ALIGNMENT_LEFT);
	add_menu_text_field(&game->game_menus.join_game_menu.menu, 2, game->game_menus.join_game_menu.port_buffer, 55, 125, -30, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, ALIGNMENT_LEFT, &game->game_menus.join_game_menu.port_box_selected, default_font);

	add_menu_label(&game->game_menus.join_game_menu.menu, 2, -123, -3, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.join_game_menu.username_text, ALIGNMENT_LEFT);
	add_menu_text_field(&game->game_menus.join_game_menu.menu, 2, game->game_menus.join_game_menu.username_buffer, -125, -5, 15, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, ALIGNMENT_LEFT, &game->game_menus.join_game_menu.username_box_selected, default_font);

	add_menu_label(&game->game_menus.join_game_menu.menu, 2, 7, -3, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.join_game_menu.password_text, ALIGNMENT_LEFT);
	add_menu_text_field(&game->game_menus.join_game_menu.menu, 2, game->game_menus.join_game_menu.password_buffer, 5, 125, 15, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, ALIGNMENT_LEFT, &game->game_menus.join_game_menu.password_box_selected, default_font);


	add_menu_label(&game->game_menus.join_game_menu.menu, 3, 0, 40, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.join_game_menu.join_game_text, ALIGNMENT_MIDDLE);
	add_menu_button(&game->game_menus.join_game_menu.menu, 2, &game->game_menus.join_game_menu.join_game_button_state, -100, 30, 100, 50, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &game->game_menus.join_game_menu.join_game_button_enabled);





	game->game_menus.connection_waiting_menu.menu.menu_items_count = 0;

	game->game_menus.connection_waiting_menu.back_button_enabled = true;
	game->game_menus.connection_waiting_menu.back_button_state = false;

	convert_string_to_gui_string_in_buffer(default_font, "Back", 1, 0xffffffff, game->game_menus.connection_waiting_menu.back_text, 5);
	convert_string_to_gui_string_in_buffer(default_font, "                                                               ", 2, 0xffffffff, game->game_menus.connection_waiting_menu.networking_message, 64);

	for(int i = 0; i < 63; i++) game->game_menus.connection_waiting_menu.networking_message[i].value = '\x1f';

	add_menu_image(&game->game_menus.connection_waiting_menu.menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 7);

	add_menu_label(&game->game_menus.connection_waiting_menu.menu, 1, 0, 0, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, game->game_menus.connection_waiting_menu.networking_message, ALIGNMENT_MIDDLE);
	
	add_menu_label(&game->game_menus.connection_waiting_menu.menu, 11, 0, -15, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, game->game_menus.connection_waiting_menu.back_text, ALIGNMENT_MIDDLE);
	add_menu_button(&game->game_menus.connection_waiting_menu.menu, 10, &game->game_menus.connection_waiting_menu.back_button_state, -100, -25, 100, -5, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, light_texture, dark_texture, &game->game_menus.connection_waiting_menu.back_button_enabled);

}


void game_menus_frame(struct game_client* game, unsigned int* pixels, int width, int height) {

	struct point2d_int mousepos = get_mouse_cursor_position(game->window);
	mousepos.x /= game->settings.resolution_scale;
	mousepos.y /= game->settings.resolution_scale;
	char click = get_key_state(KEY_MOUSE_LEFT);

	int render_gui_scale = (game->settings.gui_scale != 0 ? clamp_int(game->settings.gui_scale, 1, (width - 350) / 350 + 1) : (width - 350) / 350 + 1);

	switch (game->game_menus.active_menu) {

	case MAIN_MENU: {
		menu_scene_frame(&game->game_menus.main_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);
		if (game->game_menus.main_menu.options_button_state) {
			game->game_menus.main_menu.options_button_state = false;
			game->game_menus.active_menu = OPTIONS_MENU;
			break;
		}
		else if (game->game_menus.main_menu.join_game_button_state) {
			game->game_menus.main_menu.join_game_button_state = false;
			game->game_menus.active_menu = JOIN_GAME_MENU;
			break;
		}
		break;
	}

	case OPTIONS_MENU: {

		menu_scene_frame(&game->game_menus.options_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);

		game->settings.render_distance = (float)RENDER_DISTANCE_MIN + (game->game_menus.options_menu.render_distance_slider_state * ((float)RENDER_DISTANCE_MAX - (float)RENDER_DISTANCE_MIN));
		game->settings.fov = (float)FOV_MIN + (game->game_menus.options_menu.fov_slider_state * ((float)FOV_MAX - (float)FOV_MIN));

		if (game->game_menus.options_menu.done_button_state || get_key_state(KEY_ESCAPE) == 0b11) {
			game->game_menus.options_menu.done_button_state = false;
			game->game_menus.active_menu = MAIN_MENU;
			break;
		}

		else if (game->game_menus.options_menu.gui_scale_button_state) {
			game->game_menus.options_menu.gui_scale_button_state = false;
			game->settings.gui_scale = (game->settings.gui_scale + 1) % ((width + 350) / 350);

			if (game->settings.gui_scale > 0) {
				game->game_menus.options_menu.gui_scale_text[11].value = digit_to_char(game->settings.gui_scale);
				game->game_menus.options_menu.gui_scale_text[12].value = '\x1f';
				game->game_menus.options_menu.gui_scale_text[13].value = '\x1f';
				game->game_menus.options_menu.gui_scale_text[14].value = '\x1f';
			}
			else {
				game->game_menus.options_menu.gui_scale_text[11].value = 'A';
				game->game_menus.options_menu.gui_scale_text[12].value = 'u';
				game->game_menus.options_menu.gui_scale_text[13].value = 't';
				game->game_menus.options_menu.gui_scale_text[14].value = 'o';
			}
		}

		game->game_menus.options_menu.render_distance_text[17].value = (game->settings.render_distance < 10 ? '\x1f' : digit_to_char(game->settings.render_distance / 10));
		game->game_menus.options_menu.render_distance_text[18].value = digit_to_char(game->settings.render_distance % 10);

		game->game_menus.options_menu.fov_text[5].value = (game->settings.fov < 100 ? '\x1f' : digit_to_char(game->settings.fov / 100));
		game->game_menus.options_menu.fov_text[6].value = (game->settings.fov < 10 ? '\x1f' : digit_to_char((game->settings.fov / 10) % 10));
		game->game_menus.options_menu.fov_text[7].value = digit_to_char(game->settings.fov % 10);

		break;
	}

	case JOIN_GAME_MENU: {

		menu_scene_frame(&game->game_menus.join_game_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);

		//ip address
		if (game->game_menus.join_game_menu.ip_address_box_selected && game->game_menus.join_game_menu.ip_address_buffer_link == -1) {
			game->game_menus.join_game_menu.ip_address_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer, sizeof(game->game_menus.join_game_menu.ip_address_buffer), string_length(game->game_menus.join_game_menu.ip_address_buffer) - 1);
		}
		else if (game->game_menus.join_game_menu.ip_address_box_selected == false && game->game_menus.join_game_menu.ip_address_buffer_link != -1) {
			unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer_link);
			game->game_menus.join_game_menu.ip_address_buffer_link = -1;
		}
		//port
		if (game->game_menus.join_game_menu.port_box_selected && game->game_menus.join_game_menu.port_buffer_link == -1) {
			game->game_menus.join_game_menu.port_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer, sizeof(game->game_menus.join_game_menu.port_buffer), string_length(game->game_menus.join_game_menu.port_buffer) - 1);
		}
		else if (game->game_menus.join_game_menu.port_box_selected == false && game->game_menus.join_game_menu.port_buffer_link != -1) {
			unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer_link);
			game->game_menus.join_game_menu.port_buffer_link = -1;
		}
		//username
		if (game->game_menus.join_game_menu.username_box_selected && game->game_menus.join_game_menu.username_buffer_link == -1) {
			game->game_menus.join_game_menu.username_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.username_buffer, sizeof(game->game_menus.join_game_menu.username_buffer), string_length(game->game_menus.join_game_menu.username_buffer) - 1);
		}
		else if (game->game_menus.join_game_menu.username_box_selected == false && game->game_menus.join_game_menu.username_buffer_link != -1) {
			unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.username_buffer_link);
			game->game_menus.join_game_menu.username_buffer_link = -1;
		}
		//password
		if (game->game_menus.join_game_menu.password_box_selected && game->game_menus.join_game_menu.password_buffer_link == -1) {
			game->game_menus.join_game_menu.password_buffer_link = link_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.password_buffer, sizeof(game->game_menus.join_game_menu.password_buffer), string_length(game->game_menus.join_game_menu.password_buffer) - 1);
		}
		else if (game->game_menus.join_game_menu.password_box_selected == false && game->game_menus.join_game_menu.password_buffer_link != -1) {
			unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.password_buffer_link);
			game->game_menus.join_game_menu.password_buffer_link = -1;
		}



		if (game->game_menus.join_game_menu.back_button_state || get_key_state(KEY_ESCAPE) == 0b11) {
			game->game_menus.join_game_menu.back_button_state = false;
			game->game_menus.active_menu = MAIN_MENU;

			if (game->game_menus.join_game_menu.ip_address_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer_link);
				game->game_menus.join_game_menu.ip_address_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.port_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer_link);
				game->game_menus.join_game_menu.port_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.username_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.username_buffer_link);
				game->game_menus.join_game_menu.username_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.password_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.password_buffer_link);
				game->game_menus.join_game_menu.password_buffer_link = -1;
			}

			break;

		}

		else if (game->game_menus.join_game_menu.join_game_button_state) {
			game->game_menus.join_game_menu.join_game_button_state = false;
			game->game_menus.active_menu = CONNECTION_WAITING_MENU;

			if (game->game_menus.join_game_menu.ip_address_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.ip_address_buffer_link);
				game->game_menus.join_game_menu.ip_address_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.port_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.port_buffer_link);
				game->game_menus.join_game_menu.port_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.username_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.username_buffer_link);
				game->game_menus.join_game_menu.username_buffer_link = -1;
			}
			if (game->game_menus.join_game_menu.password_buffer_link != -1) {
				unlink_keyboard_parse_buffer(game->window, game->game_menus.join_game_menu.password_buffer_link);
				game->game_menus.join_game_menu.password_buffer_link = -1;
			}

			break;
		}

		else if (game->game_menus.join_game_menu.join_game_button_state);

		if (string_length(game->game_menus.join_game_menu.ip_address_buffer) != 1 && string_length(game->game_menus.join_game_menu.port_buffer) != 1 && string_length(game->game_menus.join_game_menu.username_buffer) != 1 && string_length(game->game_menus.join_game_menu.password_buffer) != 1) game->game_menus.join_game_menu.join_game_button_enabled = true;
		else game->game_menus.join_game_menu.join_game_button_enabled = false;


		break;
	}

	case CONNECTION_WAITING_MENU: {
		menu_scene_frame(&game->game_menus.connection_waiting_menu.menu, render_gui_scale, pixels, width, height, mousepos.x, mousepos.y, click);

		if (game->game_menus.connection_waiting_menu.back_button_state) {
			game->game_menus.connection_waiting_menu.back_button_state = false;
			game->game_menus.active_menu = JOIN_GAME_MENU;
			break;
		}

		break;
	}

	}
}