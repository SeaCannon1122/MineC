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
	uint32_t resourceFrameCount,
	VkInstance instance,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkQueue transferQueue,
	uint32_t transferQueueFamilyIndex,
	VkRenderPass renderPass,
	uint32_t subpass,
	PFN_vkGetInstanceProcAddr pfnvkGetInstanceProcAddr,
	const uint8_t* customVertexShaderSource,
	size_t customVertexShaderSourceSize,
	const uint8_t* customFragmentShaderSource,
	size_t customFragmentShaderSourceSize
);

void pixelcharRendererBackendVulkanDeinitialize(PixelcharRenderer renderer, uint32_t backendSlotIndex);

PixelcharResult pixelcharRendererBackendVulkanUpdateRenderingData(PixelcharRenderer renderer, uint32_t backendSlotIndex, uint32_t resourceFrameIndex, VkCommandBuffer commandBuffer);

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
