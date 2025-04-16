#pragma once

#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <stdint.h>

#include "application_window.h"

enum gpu_type 
{
	GPU_TYPE_GPU = 2,
	GPU_TYPE_VIRTUAL_GPU = 3,
	GPU_TYPE_IGPU = 1,
	GPU_TYPE_CPU = 4,
	GPU_TYPE_UNKOWN = 0,
};


struct application_state 
{
	void* string_allocator;

	struct application_window main_window;

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

	struct _machine_info {
		struct _gpus {
			uint8_t name[64];
			uint32_t usable;
			uint32_t vulkan_version_major;
			uint32_t vulkan_version_minor;
			uint32_t vulkan_version_patch;
			uint32_t gpu_type;
		} gpus[16];
		uint32_t gpu_count;
		uint32_t gpu_index;
	} _machine_info;

	double time;

};

uint32_t application_create(struct application_state* app);

uint32_t application_handle_events(struct application_state* app);

uint32_t application_destroy(struct application_state* app);

#endif // !APPLICATION_STATE_H