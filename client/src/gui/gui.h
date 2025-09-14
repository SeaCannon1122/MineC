#pragma once

#ifndef MINEC_CLIENT_GUI_GUI_H
#define MINEC_CLIENT_GUI_GUI_H

#include <stdbool.h>
#include <mutex.h>

#include <stdint.h>

enum gui_char_flag_bit
{
	GUI_CHAR_FLAG_BIT_SHADOW,
	GUI_CHAR_FLAG_BIT_BACKGROUND,
	GUI_CHAR_FLAG_BIT_BOLD,
	GUI_CHAR_FLAG_BIT_ITALIC,
	GUI_CHAR_FLAG_BIT_UNDERLINE,
	GUI_CHAR_FLAG_BIT_STRIKETHROUGH,
	GUI_CHAR_FLAG_BIT_OBFUSCATED
};

struct gui_char
{
	uint32_t character;
	uint32_t flags;
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
	struct gui_char text[MENU_GUI_LABEL_MAX_TEXT_BUFFER_LENGTH];
};

struct menu_gui_item_icon_button_create_imfo
{
	bool disabled;
	uint8_t asset_path[MENU_GUI_ICON_BUTTON_MAX_ASSET_PATH_LENGTH];
};

struct menu_gui_item_label_create_imfo
{
	struct gui_char text[MENU_GUI_LABEL_MAX_TEXT_BUFFER_LENGTH];
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

struct gui_state
{
	int a;
};

#endif
