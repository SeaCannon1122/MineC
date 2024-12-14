#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <stdint.h>

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

	struct {
		uint32_t width;
		uint32_t height;
	} window_extent;

	struct {
		uint32_t x;
		uint32_t y;
	} window_position;

	struct {
		struct {
			uint8_t name[256];
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
	
};

uint32_t application_create(struct game_client* game);

uint32_t application_handle_events(struct game_client* game);

uint32_t application_destroy(struct game_client* game);

#endif // !APPLICATION_STATE_H