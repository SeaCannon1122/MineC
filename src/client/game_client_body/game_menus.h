#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "client/gui/menu.h"
#include "client/game_client_body/resources.h"
#include "game/game_constants.h"

#include "general/keyvalue.h"

#define main_menu_this_is_not_minecraft_label_text "This is not Minecraft!!!"
#define main_menu_join_game_label_text "Join Game"
#define main_menu_options_label_text "Options..."
#define main_menu_quit_game_label_text "Quit Game"

#define options_menu_options_label_text "Options"
#define options_menu_fov_label_text "FOV:    "
#define options_menu_render_distance_label_text "Render Distance:    Chunks"
#define options_menu_gui_scale_label_text "GUI Scale:     "
#define options_menu_done_label_text "Done"

enum blocks_menus {
	NO_MENU,
	MAIN_MENU,
	OPTIONS_MENU,
	JOIN_GAME_MENU,
	CONNECTION_WAITING_MENU,
	CHAT_MENU,
	INGAME_OPTIONS_MENU,
};

struct game_menu_label {
	int menu_item_type;
	int z;
	int x;
	int y;
	int alignment_x;
	int alignment_y;
	int text_alignment_x;
	int text_alignment_y;
	int max_width;
	int max_rows;
	int selectable;
	int hoverable;
	int text_size;
	struct pixel_char text[64];
};

struct game_menus {
	struct main_menu {

		struct game_menu_label this_is_not_minecraft_label;
		struct game_menu_label join_game_label;
		struct game_menu_label options_label;
		struct game_menu_label quit_game_label;

		struct menu_image background_image;

		struct menu_image join_game_button_image;
		struct menu_image options_button_image;
		struct menu_image quit_game_button_image;

		struct main_menu_scene {
			int current_item;

			int select_label;
			int select_begin;
			int select_end;
			int selecting;

			int current_pos;

			int image_pos_x;
			int image_pox_y;
			int image_index;

			int items_count;
			int* items[20];
		} menu;
	} main_menu;

	struct options_menu {
		struct game_menu_label options_label;
		struct game_menu_label fov_label;
		struct game_menu_label render_distance_label;
		struct game_menu_label gui_scale_label;
		struct game_menu_label done_label;

		struct menu_image background_image;

		struct menu_image gui_scale_button_image;
		struct menu_image done_button_image;

		struct options_menu_scene {
			int select_label;
			int current_pos;
			int select_begin;
			int select_end;
			int selecting;

			int items_count;
			int* items[20];
		} menu;
	} options_menu;

	/*struct join_game_menu {
		struct gui_character join_a_multiplayer_game_text[24];

		struct gui_character ip_address_text[12];

		char ip_address_buffer[15 + 1];
		bool ip_address_box_selected;
		int ip_address_buffer_link;
		bool ip_address_field_visible;

		struct gui_character port_text[5 + 1];

		char port_buffer[5 + 1];
		bool port_box_selected;
		int port_buffer_link;
		bool port_field_visible;

		struct gui_character username_text[10];

		char username_buffer[MAX_USERNAME_LENGTH + 1];
		bool username_box_selected;
		int username_buffer_link;
		bool username_field_visible;

		struct gui_character password_text[10];

		char password_buffer[MAX_PASSWORD_LENGTH + 1];
		bool password_box_selected;
		int password_buffer_link;
		bool password_field_visible;

		struct gui_character join_game_text[10];
		bool join_game_button_state;
		bool join_game_button_enabled;

		bool back_button_state;
		bool back_button_enabled;
		struct gui_character back_text[5];

		struct menu_scene menu;
	} join_game_menu;

	struct {

		char networking_message[CONECTION_CLOSE_MESSAGE_LENGTH + 1];
		struct gui_character networking_message_gui[CONECTION_CLOSE_MESSAGE_LENGTH + 1];

		bool back_button_state;
		bool back_button_enabled;
		struct gui_character back_text[5];

		struct menu_scene menu;
	} connection_waiting_menu;

	struct {

		char message_buffer[MAX_CHAT_MESSAGE_LENGTH + 1];
		int message_link;
		struct char_font* font;

		struct menu_scene menu;
	} chat_menu;

	struct {
		struct gui_character options_text[8];

		bool back_to_game_button_state;
		bool back_to_game_button_enabled;
		struct gui_character back_to_game_text[13];

		float fov_slider_state;
		struct gui_character fov_text[9];

		float render_distance_slider_state;
		struct gui_character render_distance_text[27];

		bool gui_scale_button_state;
		bool gui_scale_button_enabled;
		struct gui_character gui_scale_text[16];

		bool disconnect_button_state;
		bool disconnect_button_enabled;
		struct gui_character disconnect_text[11];

		struct menu_scene menu;
	} in_game_options_menu;*/

	int active_menu;
};

struct game_client;

void init_game_menus(struct game_client* game);

void switch_game_menu(struct game_client* game, int menu);

void game_menus_frame(struct game_client* game);
