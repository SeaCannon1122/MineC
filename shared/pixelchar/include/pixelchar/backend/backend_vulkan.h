#pragma once

#ifndef PIXELCHAR_BACKEND_VULKANL_H
#define PIXELCHAR_BACKEND_VULKANL_H

#include <pixelchar/pixelchar.h>

#include <vulkan/vulkan.h>

PixelcharResult pixelcharRendererBackendVulkanInitialize(
	PixelcharRenderer renderer, 
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue queue,
	uint32_t queueIndex,
	VkRenderPass renderPass,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer);

PixelcharResult pixelcharRendererBackendVulkanRender(
	PixelcharRenderer renderer,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);

#endif
