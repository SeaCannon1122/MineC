#pragma once

#ifndef MINEC_CLIENT_GUI_GUI_H
#define MINEC_CLIENT_GUI_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include <mutex.h>

enum menu_gui_item_type
{
	MENU_GUI_ITEM_TYPE_BUTTON,
};

#define MENU_GUI_BUTTON_MAX

struct menu_gui_item_button
{
	bool disabled;

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
	
	union
	{
		struct menu_gui_item_button button;
	};
};

struct menu_gui
{
	mutex_t mutex;
	uint32_t item_count;
	struct menu_gui_item items[];
};

struct gui_state
{
	int a;
};

#endif
