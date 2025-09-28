#pragma once

#ifndef MINEC_CLIENT_GUI_GUI_H
#define MINEC_CLIENT_GUI_GUI_H

#include <stdbool.h>
#include <mutex.h>

#include <stdint.h>



enum guichar_modifier
{
	GUICHAR_MODIFIER_BOLD_BIT = 1,
	GUICHAR_MODIFIER_ITALIC_BIT = 2,
	GUICHAR_MODIFIER_UNDERLINE_BIT = 4,
	GUICHAR_MODIFIER_STRIKETHROUGH_BIT = 8,
	GUICHAR_MODIFIER_SHADOW_BIT = 16,
	GUICHAR_MODIFIER_BACKGROUND_BIT = 32,
};

struct guichar
{
	uint32_t character;
	uint32_t modifiers;
	uint32_t color;
	uint32_t background_color;
};

enum menu_gui_item_type
{
	MENU_GUI_ITEM_TYPE_BUTTON,
	MENU_GUI_ITEM_TYPE_SMALL_BUTTON,
	MENU_GUI_ITEM_TYPE_ICON_BUTTON,
	MENU_GUI_ITEM_TYPE_LABEL,
	MENU_GUI_ITEM_TYPE_TEXTFIELD,
	MENU_GUI_ITEM_TYPE_IMAGE,
};

#define MENU_GUI_BUTTON_WIDTH 200
#define MENU_GUI_BUTTON_HEIGHT 20

#define MENU_GUI_LABEL_MAX_TEXT_BUFFER_LENGTH 64
#define MENU_GUI_ICON_BUTTON_MAX_ASSET_PATH_LENGTH 256


#define MENU_GUI_TEXTFIELD_MAX_BUFFER_LENGTH 256

struct menu_gui_item_button_create_imfo
{
	bool disabled;
	struct guichar text[MENU_GUI_LABEL_MAX_TEXT_BUFFER_LENGTH];
};

struct menu_gui_item_icon_button_create_imfo
{
	bool disabled;
	uint8_t asset_path[MENU_GUI_ICON_BUTTON_MAX_ASSET_PATH_LENGTH];
};

struct menu_gui_item_label_create_imfo
{
	struct guichar text[MENU_GUI_LABEL_MAX_TEXT_BUFFER_LENGTH];
};

struct menu_gui_item_textfield_create_imfo
{
	bool disabled;
	uint32_t text_buffer_length;
};

struct menu_gui_item_image_create_info
{
	uint8_t asset_path[MENU_GUI_ICON_BUTTON_MAX_ASSET_PATH_LENGTH];
	uint32_t width;
	uint32_t height;
};

struct menu_gui_item_create_info
{
	uint32_t type;
	struct
	{
		int32_t x;
		int32_t y;
		int32_t z;
	} position;
	struct
	{
		float width;
		float height;
	} relative_position;

	union
	{
		struct menu_gui_item_button_create_imfo button;
		struct menu_gui_item_label_create_imfo label;
		struct menu_gui_item_textfield_create_imfo textfield;
		struct menu_gui_item_image_create_info image;
	};
};

struct menu_gui_item
{
	uint32_t type;
	struct
	{
		int32_t x;
		int32_t y;
		int32_t z;
	} position;
	struct
	{
		float width;
		float height;
	} relative_position;

	/*union
	{
		struct menu_gui_item_button button;
		struct menu_gui_item_label label;
		struct menu_gui_item_textfield textfield;
		struct menu_gui_item_image image;
	};*/
};

struct menu_gui
{
	mutex_t mutex;
	uint32_t item_count;
	struct menu_gui_item* items;
};

void pixelfont_load(struct minec_client* client);

//pixelfont

#define GUI_STATE client->gui_state

struct gui_state
{
	struct
	{
		void* data;
		
		uint32_t resolution;

		uint32_t mapping_count;
		uint32_t* mappings;

		uint32_t bitmap_count;
		struct bitmap_metadata* metadata;
		uint32_t* bitmap_data;
	} font;
};

void gui_state_reload_assets(struct minec_client* client);
uint32_t gui_state_create(struct minec_client* client);
void gui_state_destroy(struct minec_client* client);

#endif
