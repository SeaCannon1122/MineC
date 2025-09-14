#include <minec_client.h>

#define german_language_info { .name = U"English", .country = U"Deutschland" }

void string_index_german_init(void* hashmap)
{
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_SINGLEPLAYER_STRING",	U"Einzelspieler",	HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_MULTIPLAYER_STRING",	U"Mehrspieler",		HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_SETTINGS_STRING",		U"Einstellungen",	HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_CREDITS_STRING",		U"Credits",			HASHMAP_VALUE_STRING32);
	hashmap_set_value(hashmap, "MENU_GUI_MAIN_BUTTON_QUITGAME_STRING",		U"Spiel beenden",	HASHMAP_VALUE_STRING32);


	hashmap_set_value(hashmap, "MENU_GUI_SETTINGS_TITLE_STRING",	U"Einstellungen",	HASHMAP_VALUE_STRING32);
}