#include "game_menus.h"

#include "general/keyvalue.h"
#include "general/utils.h"
#include "client/gui/char_font.h"
#include "client/gui/menu.h"
#include "game_client.h"


void init_game_menus(struct game_client* game) {

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

	game->game_menus.main_menu.this_is_not_minecraft_text = convert_string_to_gui_string(default_font, "This is not Minecraft!!!", 4);
	game->game_menus.main_menu.join_game_text = convert_string_to_gui_string(default_font, "Join Game", 1);
	game->game_menus.main_menu.options_text = convert_string_to_gui_string(default_font, "Options...", 1);
	game->game_menus.main_menu.quit_game_text = convert_string_to_gui_string(default_font, "Quit Game", 1);

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
	game->game_menus.options_menu.fov_slider_state = ((float)game->settings.fov) / ((float)FOV_MAX - (float)FOV_MIN);
	game->game_menus.options_menu.render_distance_slider_state = ((float)game->settings.render_distance - (float)RENDER_DISTANCE_MIN) / ((float)RENDER_DISTANCE_MAX - (float)RENDER_DISTANCE_MIN);
	game->game_menus.options_menu.gui_scale_button_enabled = true;
	game->game_menus.options_menu.gui_scale_button_state = false;
	

	game->game_menus.options_menu.options_text = convert_string_to_gui_string(default_font, "Options", 2);
	game->game_menus.options_menu.fov_text = convert_string_to_gui_string(default_font, "FOV:    ", 1);
	game->game_menus.options_menu.render_distance_text = convert_string_to_gui_string(default_font, "Render Distance:    Chunks", 1);
	game->game_menus.options_menu.render_distance_text[17].value = digit_to_char(game->settings.render_distance / 10);
	game->game_menus.options_menu.render_distance_text[18].value = digit_to_char(game->settings.render_distance % 10);
	game->game_menus.options_menu.gui_scale_text = convert_string_to_gui_string(default_font, "GUI Scale:  ", 1);
	game->game_menus.options_menu.gui_scale_text[11].value = digit_to_char(game->settings.gui_scale);

	game->game_menus.options_menu.done_text = convert_string_to_gui_string(default_font, "Done", 1);

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

	game->game_menus.join_game_menu.join_game_button_enabled = true;
	game->game_menus.join_game_menu.join_game_button_state = false;
	game->game_menus.join_game_menu.back_button_state = false;
	game->game_menus.join_game_menu.back_button_enabled = true;
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.ip_address_buffer); i++) game->game_menus.join_game_menu.ip_address_buffer[i] = '\0';
	for (int i = 0; i < sizeof(game->game_menus.join_game_menu.port_buffer); i++) game->game_menus.join_game_menu.port_buffer[i] = '\0';

	game->game_menus.join_game_menu.back_text = convert_string_to_gui_string(default_font, "Back", 1);
	game->game_menus.join_game_menu.join_a_multiplayer_game_text = convert_string_to_gui_string(default_font, "Join a multiplayer game", 2);

	add_menu_image(&game->game_menus.join_game_menu.menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 7);

	add_menu_label(&game->game_menus.join_game_menu.menu, 0, 0, 25, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, game->game_menus.join_game_menu.join_a_multiplayer_game_text, ALIGNMENT_MIDDLE);

	add_menu_label(&game->game_menus.join_game_menu.menu, 10, 0, -15, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, game->game_menus.join_game_menu.back_text, ALIGNMENT_MIDDLE);
	add_menu_button(&game->game_menus.join_game_menu.menu, 2, &game->game_menus.join_game_menu.back_button_state, -100, -25, 100, -5, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, light_texture, dark_texture, &game->game_menus.join_game_menu.back_button_enabled);


}