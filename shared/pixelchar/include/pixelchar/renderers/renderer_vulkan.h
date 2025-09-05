#pragma once

#ifndef PIXELCHAR_BACKEND_VULKANL_H
#define PIXELCHAR_BACKEND_VULKANL_H


#include <pixelchar/pixelchar.h>
#include <vulkan/vulkan.h>

#define PIXELCHAR_RENDERER_VULKAN_MAX_RESOURCE_FRAME_COUNT 4

#define PIXELCHAR_RENDERER_VULKAN_MAX_RENDERPASS_COUNT 8
#define PIXELCHAR_RENDERER_VULKAN_MAX_SUBPASS_COUNT 8

typedef struct PixelcharRendererVulkanRenderPassInfo
{
	VkRenderPass renderPass;
	uint32_t subPasses[PIXELCHAR_RENDERER_VULKAN_MAX_SUBPASS_COUNT];
	uint32_t subPassCount;
} PixelcharRendererVulkanRenderPassInfo;

typedef struct PixelcharRendererVulkanCreateInfo
{
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
	uint32_t queueFamilyIndex;
	PixelcharRendererVulkanRenderPassInfo* pRenderPassInfos;
	uint32_t renderPassInfoCount;
	PFN_vkGetInstanceProcAddr pfnvkGetInstanceProcAddr;
	const uint8_t* pCustomVertexShaderSource;
	size_t customVertexShaderSourceSize;
	const uint8_t* pCustomFragmentShaderSource;
	size_t customFragmentShaderSourceSize;
	uint32_t resourceFrameCount;
	uint32_t maxResourceFrameCharacterCount;
} PixelcharRendererVulkanCreateInfo;

typedef struct PixelcharRendererVulkan_T* PixelcharRendererVulkan;

VkResult PixelcharRendererVulkanCreate(PixelcharRendererVulkanCreateInfo* pInfo, PixelcharRendererVulkan* pRenderer);
void PixelcharRendererVulkanDestroy(PixelcharRendererVulkan renderer);
VkResult PixelcharRendererVulkanUseFont(PixelcharRendererVulkan renderer, PixelcharFont font, uint32_t fontIndex);

VkResult PixelcharRendererVulkanUpdateRenderingData(
	PixelcharRendererVulkan renderer, 
	Pixelchar* pCharacters, 
	uint32_t characterCount, 
	uint32_t resourceFrameIndex, 
	VkCommandBuffer commandBuffer
);

void PixelcharRendererVulkanResetResourceFrame(PixelcharRendererVulkan renderer, uint32_t resourceFrameIndex);

void PixelcharRendererVulkanRender(
	PixelcharRendererVulkan renderer,
	VkRenderPass renderPass,
	uint32_t subPass,
	VkCommandBuffer commandBuffer,
	uint32_t width,
	uint32_t height,
	float shadowDevisorR,
	float shadowDevisorG,
	float shadowDevisorB,
	float shadowDevisorA
);

#ifdef __cplusplus.
}
#endif

#endif
