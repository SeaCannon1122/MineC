#include <minec_client.h>

#define english_language_info { .name = U"English", .country = U"US" }

void string_index_english_init(void* hashmap)
{
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_SINGLEPLAYER_STRING",	U"Singleplayer",	HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_MULTIPLAYER_STRING",	U"Multiplayer",		HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_SETTINGS_STRING",		U"Settings",		HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_CREDITS_STRING",		U"Credits",			HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_QUITGAME_STRING",		U"Quit Game",		HASHMAP_VALUE_STRING32);


	hashmap_set_value(hashmap, "MENU_GUI_SETTINGS_TITLE_STRING",	U"Settings",	HASHMAP_VALUE_STRING32);
}