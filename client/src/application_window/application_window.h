#pragma once

#ifndef APPLICATION_WINDOW
#define APPLICATION_WINDOW

#include <stdint.h>
#include <window/window.h>

#define MAX_FRAME_CHAR_INPUTS 16

#define KEY_CHANGE_MASK 0b10
#define KEY_DOWN_MASK  0b01

struct application_window
{
	void* window_handle;

	void* rendering_backend_handle;

	atomic_(uint32_t) width;
	atomic_(uint32_t) height;

	struct application_window_input {
		uint32_t characters[MAX_FRAME_CHAR_INPUTS];
		uint32_t character_count;

		uint8_t keyboard[WINDOW_KEY_TOTAL_COUNT];

		uint32_t mouse_x;
		uint32_t mouse_y;

		int32_t mouse_scroll_steps;

	} input;
};

struct minec_client;

uint32_t application_window_create(struct minec_client* client, uint32_t posx, uint32_t posy, uint32_t width, uint32_t height, uint8_t* name);

uint32_t application_window_handle_events(struct minec_client* client);

uint32_t application_window_destroy(struct minec_client* client);

#endif // !APPLICATION_WINDOW
