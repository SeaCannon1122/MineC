#pragma once

#ifndef APPLICATION_WINDOW
#define APPLICATION_WINDOW

#include <stdint.h>

#include "general/platformlib/platform/platform.h"

#define MAX_FRAME_CHAR_INPUTS 16

#define KEY_CHANGE_MASK 0b10
#define KEY_DOWN_MASK  0b01

enum frame_flags
{
	FRAME_FLAG_WINDOW_SELECTED = 1,
	FRAME_FLAG_RENDERABLE = 2,
	FRAME_FLAG_RESIZE = 4,
	FRAME_FLAG_MOUSE_MOVE = 8,
};

struct application_window
{
	uint32_t window_handle;

	void* rendering_backend_handle;

	uint32_t width;
	uint32_t height;

	uint32_t last_render_width;
	uint32_t last_render_height;

	uint32_t frame_flags;

	struct application_window_input {
		uint32_t characters[MAX_FRAME_CHAR_INPUTS];
		uint32_t character_count;

		uint8_t keyboard[KEY_TOTAL_COUNT];

		uint32_t mouse_x;
		uint32_t mouse_y;

		int32_t mouse_scroll_steps;

	} input;
};

uint32_t application_window_create(struct application_window* window, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name);

uint32_t application_window_handle_events(struct application_window* window);

uint32_t application_window_destroy(struct application_window* window);

#endif // !APPLICATION_WINDOW
