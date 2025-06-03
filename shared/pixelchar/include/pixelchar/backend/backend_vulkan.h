#pragma once

#ifndef PIXELCHAR_BACKEND_VULKANL_H
#define PIXELCHAR_BACKEND_VULKANL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pixelchar/pixelchar.h>

#include <vulkan/vulkan.h>

PixelcharResult pixelcharRendererBackendVulkanInitialize(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue queue,
	uint32_t queueIndex,
	VkRenderPass renderPass,
	uint32_t subpass,
	PFN_vkGetDeviceProcAddr pfnvkGetDeviceProcAddr,
	uint8_t* vertex_shader_custom,
	uint32_t vertex_shader_custom_length,
	uint8_t* fragment_shader_custom,
	uint32_t fragment_shader_custom_length
);

void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer, uint32_t backendSlotIndex);

PixelcharResult pixelcharRendererBackendVulkanUpdateRenderingData(PixelcharRenderer renderer, uint32_t backendSlotIndex, VkCommandBuffer commandBuffer);

PixelcharResult pixelcharRendererBackendVulkanRender(
	PixelcharRenderer renderer,
	uint32_t backendSlotIndex,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);

#ifdef __cplusplus
}
#endif

#endif
