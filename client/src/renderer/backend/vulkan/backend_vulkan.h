#pragma once

#ifndef MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H
#define MINEC_CLIENT_RENDERER_BACKEND_VULKAN_BACKEND_VULKAN_H

#include <cwindow/cwindow.h>

struct renderer_backend_vulkan_base {

	int a;
};

struct renderer_backend_vulkan_device {
	uint32_t resource_frame_index;
};

struct renderer_backend_vulkan_swapchain {
	int a;
};

struct renderer_backend_info* renderer_backend_vulkan_get_info(
	struct minec_client* client
);

uint32_t renderer_backend_vulkan_base_create(
	struct minec_client* client,
	cwindow_context* window_context,
	cwindow* window,
	struct renderer_backend_device_infos* device_infos,
	struct renderer_backend_base* base
);

void renderer_backend_vulkan_base_destroy(
	struct minec_client* client,
	struct renderer_backend_base* base
);

uint32_t renderer_backend_vulkan_device_create(
	struct minec_client* client,
	uint32_t device_index,
	struct renderer_backend_device* device
);

void renderer_backend_vulkan_device_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device
);

uint32_t renderer_backend_vulkan_swapchain_create(
	struct minec_client* client,
	struct renderer_backend_device* device,
	bool vsync,
	bool triple_buffering,
	struct renderer_backend_swapchain* swapchain
);

void renderer_backend_vulkan_swapchain_destroy(
	struct minec_client* client,
	struct renderer_backend_device* device,
	struct renderer_backend_swapchain* swapchain
);

uint32_t renderer_backend_vulkan_frame_start(
	struct minec_client* client,
	struct renderer_backend_device* device
);

uint32_t renderer_backend_vulkan_frame_submit(
	struct minec_client* client,
	struct renderer_backend_device* device
);

#endif
