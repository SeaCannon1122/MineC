#pragma once

#ifndef PIXELCHAR_FONT_VULKAN_BACKEND_H
#define PIXELCHAR_FONT_VULKAN_BACKEND_H

#include <vulkan/vulkan.h>

struct pixelchar_font_backend_vulkan
{
	uint32_t references;

	VkDevice device;

	VkBuffer buffer;
	VkDeviceMemory memory;
};

#ifndef _PIXELCHAR_INTERNAL_EXCLUDE

uint32_t _pixelchar_font_backend_vulkan_reference_add(struct pixelchar_font* font, struct pixelchar_renderer* pcr, uint32_t index);

void _pixelchar_font_backend_vulkan_reference_subtract(struct pixelchar_font* font);

void _pixelchar_font_backend_vulkan_reference_init(struct pixelchar_font* font);

#endif

#endif
