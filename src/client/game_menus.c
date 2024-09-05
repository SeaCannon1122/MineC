#include "game_menus.h"

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "general/utils.h"
#include "general/resource_manager.h"
#include "client/gui/char_font.h"
#include "gui/menu.h"
#include "general/networking/client.h"


void init_main_menu(struct menu_scene* menu, struct main_menu_flags* flags) {

	menu->menu_items_count = 0;

	struct argb_image* background = get_resource("menu_background");

	struct argb_image* light_texture = get_resource("menu_button_enabled");
	struct argb_image* dark_texture = get_resource("menu_button_disabled");

	struct char_font* default_font = get_resource("default_pixelfont");

	struct gui_character* not_minecraft_text_gui = convert_string_to_gui_string(default_font, "Not Minecraft", 4);
	struct gui_character* jg_text_gui = convert_string_to_gui_string(default_font, "Join Game", 1);
	struct gui_character* options_text_gui = convert_string_to_gui_string(default_font, "Options...", 1);
	struct gui_character* quit_text_gui = convert_string_to_gui_string(default_font, "Quit Game", 1);

	add_menu_image(menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 10);

	add_menu_label(menu, 10, 0, 60, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, not_minecraft_text_gui, ALIGNMENT_MIDDLE);
	add_menu_label(menu, 10, 0, 0, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, jg_text_gui, ALIGNMENT_MIDDLE);
	add_menu_label(menu, 10, -49, 60, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, options_text_gui, ALIGNMENT_MIDDLE);
	add_menu_label(menu, 10, 49, 60, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, quit_text_gui, ALIGNMENT_MIDDLE);

	add_menu_button(menu, 1, &flags->join_game, -100, -10, 100, 10, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &flags->join_game_enable);
	add_menu_button(menu, 1, &flags->options, -100, 50, -2, 70, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &flags->options_enable);
	add_menu_button(menu, 1, &flags->quit_game, 2, 50, 100, 70, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, light_texture, dark_texture, &flags->quit_game_enable);
}

void init_options_menu(struct menu_scene* menu, struct options_menu_flags* flags) {

	menu->menu_items_count = 0;

	struct argb_image* background = get_resource("menu_background");

	struct argb_image* light_texture = get_resource("menu_button_enabled");
	struct argb_image* dark_texture = get_resource("menu_button_disabled");

	struct char_font* default_font = get_resource("default_pixelfont");

	struct gui_character* options_text_gui = convert_string_to_gui_string(default_font, "Options", 1);

	struct gui_character* done_text_gui = convert_string_to_gui_string(default_font, "Done", 1);

	add_menu_image(menu, -1, 0, 0, ALIGNMENT_LEFT, ALIGNMENT_TOP, ALIGNMENT_LEFT, ALIGNMENT_TOP, background, 10);
	add_menu_label(menu, 10, 0, 15, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, options_text_gui, ALIGNMENT_MIDDLE);


	add_menu_slider(menu, 1, &flags->fov, -154, 50, -4, 70, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, dark_texture, light_texture, 10);
	add_menu_button(menu, 1, &flags->gui_scale, 4, 50, 154, 70, ALIGNMENT_MIDDLE, ALIGNMENT_TOP, light_texture, dark_texture, &flags->gui_scale_enable);
	
	//add_menu_button(menu, 1, &flags->done, -100, -10, 100, 10, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, button_texture_enabled, button_texture_disabled, &flags->done_enable);
	//add_menu_button(menu, 1, &flags->done, -100, -10, 100, 10, ALIGNMENT_MIDDLE, ALIGNMENT_MIDDLE, button_texture_enabled, button_texture_disabled, &flags->done_enable);

	add_menu_label(menu, 10, 0, -15, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, done_text_gui, ALIGNMENT_MIDDLE);
	add_menu_button(menu, 2, &flags->done, -100, -25, 100, -5, ALIGNMENT_MIDDLE, ALIGNMENT_BOTTOM, light_texture, dark_texture, &flags->done_enable);

}