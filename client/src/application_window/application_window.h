#pragma once

#ifndef MINEC_CLIENT_APPLICATION_WINDOW_APPLICATION_WINDOW_H
#define MINEC_CLIENT_APPLICATION_WINDOW_APPLICATION_WINDOW_H

#include <stdint.h>
#include <cwindow/cwindow.h>

#define MAX_FRAME_CHAR_INPUTS 16

#define KEY_CHANGE_MASK 0b10
#define KEY_DOWN_MASK  0b01

#define APPLICATION_WINDOW client->window

struct application_window
{
	cwindow_context* context;
	cwindow* window;

	atomic_uint32_t width;
	atomic_uint32_t height;

	struct application_window_input {
		uint32_t characters[MAX_FRAME_CHAR_INPUTS];
		uint32_t character_count;

		uint8_t keyboard[CWINDOW_KEY_TOTAL_COUNT];

		uint32_t mouse_x;
		uint32_t mouse_y;

		int32_t mouse_scroll_steps;

	} input;
};

#ifndef MINEC_CLIENT_INCLUDE_ONLY_STRUCTURE

struct minec_client;

uint32_t application_window_create(struct minec_client* client);
uint32_t application_window_destroy(struct minec_client* client);

uint32_t application_window_events(struct minec_client* client);


#endif

#endif
