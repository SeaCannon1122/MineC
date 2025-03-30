#pragma once

#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <stdint.h>

#define MAX_FRAME_CHAR_INPUTS 16

#define KEY_CHANG_MASK 0b10
#define KEY_DOWN_MASK  0b01

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

	struct last_render_window_extent {
		uint32_t width;
		uint32_t height;
	} last_render_window_extent;

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

		uint8_t keyboard[KEY_TOTAL_COUNT];

		struct mouse_coords {
			uint32_t x;
			uint32_t y;
		} mouse_coords;

	} input_state;

	double time;

};

struct game_client;

uint32_t application_create(struct game_client* game);

uint32_t application_handle_events(struct game_client* game);

uint32_t application_destroy(struct game_client* game);

#endif // !APPLICATION_STATE_H