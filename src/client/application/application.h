#pragma once

#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <stdint.h>

#define MAX_FRAME_CHAR_INPUTS 64

enum gpu_type {
	GPU_TYPE_GPU = 2,
	GPU_TYPE_VIRTUAL_GPU = 3,
	GPU_TYPE_IGPU = 1,
	GPU_TYPE_CPU = 4,
	GPU_TYPE_UNKOWN = 0,
};

enum frame_flags {
	FRAME_FLAG_SIZE_CHANGE = 1,
	FRAME_FLAG_SHOULD_RENDER = 2,
};

struct application_state {
	uint32_t window;

	struct window_extent {
		uint32_t width;
		uint32_t height;
	} window_extent;

	struct window_position {
		uint32_t x;
		uint32_t y;
	} window_position;

	struct machine_info {
		struct gpus {
			uint8_t name[64];
			uint32_t usable;
			uint32_t vulkan_version_major;
			uint32_t vulkan_version_minor;
			uint32_t vulkan_version_patch;
			uint32_t gpu_type;
		} gpus[16];
		uint32_t gpu_count;
		uint32_t gpu_index;
	} machine_info;

	uint32_t frame_flags;
	
	struct input_state {
		uint32_t characters[MAX_FRAME_CHAR_INPUTS];
		uint32_t character_count;

		struct keyboard {
			uint8_t A;
			uint8_t B;
			uint8_t C;
			uint8_t D;
			uint8_t E;
			uint8_t F;
			uint8_t G;
			uint8_t H;
			uint8_t I;
			uint8_t J;
			uint8_t K;
			uint8_t L;
			uint8_t M;
			uint8_t N;
			uint8_t O;
			uint8_t P;
			uint8_t Q;
			uint8_t R;
			uint8_t S;
			uint8_t T;
			uint8_t U;
			uint8_t V;
			uint8_t W;
			uint8_t X;
			uint8_t Y;
			uint8_t Z;
			uint8_t SPACE;
			uint8_t ENTER;
			uint8_t NUM1;
			uint8_t NUM2;
			uint8_t NUM3;
			uint8_t NUM4;
			uint8_t NUM5;
			uint8_t NUM6;
			uint8_t NUM7;
			uint8_t NUM8;
			uint8_t NUM9;
			uint8_t NUM0;
			uint8_t ARROW_UP;
			uint8_t ARROW_DOWN;
			uint8_t ARROW_LEFT;
			uint8_t ARROW_RIGHT;
			uint8_t F1;
			uint8_t F2;
			uint8_t F3;
			uint8_t F4;
			uint8_t F5;
			uint8_t F6;
			uint8_t F7;
			uint8_t F8;
			uint8_t F9;
			uint8_t F10;
			uint8_t F11;
			uint8_t F12;
			uint8_t ESC;
			uint8_t SHIFT;
			uint8_t TAB;
			uint8_t CTRL;
			
		} keyboard;

		struct mouse_coords {
			uint32_t x;
			uint32_t y;
		} mouse_coords;

	} input_state;

};

struct game_client;

uint32_t application_create(struct game_client* game);

uint32_t application_handle_events(struct game_client* game);

uint32_t application_destroy(struct game_client* game);

#endif // !APPLICATION_STATE_H