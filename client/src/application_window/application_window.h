#pragma once

#ifndef APPLICATION_WINDOW
#define APPLICATION_WINDOW

#include <stdint.h>
#include <window/window.h>

#define MAX_FRAME_CHAR_INPUTS 16

#define KEY_CHANGE_MASK 0b10
#define KEY_DOWN_MASK  0b01

#define WINDOW client->window

struct application_window
{
	void* window_handle;
	void* window_context_handle;

	atomic_uint32_t width;
	atomic_uint32_t height;

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

#ifndef MINEC_CLIENT_ONLY_RUNTIME_INTERFACE
uint32_t application_window_create(struct minec_client* client);
uint32_t application_window_destroy(struct minec_client* client);

uint32_t application_window_handle_events(struct minec_client* client);
#endif

void application_window_get_dimensions(struct minec_client* client, uint32_t* width, uint32_t* height);


#endif // !APPLICATION_WINDOW
