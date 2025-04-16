#pragma once

#ifndef RESOURCES_H
#define RESOURCES_H

#include "stdbool.h"
#include <pixelchar/pixelchar.h>

struct resources_image
{
	struct
	{
		uint32_t width;
		uint32_t height;
		void* data;
	} image;

	bool in_use;
};

struct resources_pixelchar_font
{
	struct pixelchar_font font;

	bool in_use;
};

struct resources_key_value_map
{
	void* map;

	bool in_use;
};

struct resources_shader
{
	struct
	{
		void* data;
		size_t data_size;
	} shader;

	bool in_use;
};

struct resources_state {


};

#endif // !RESOURCES_H
