#pragma once

#ifndef PIXELCHAR_IMPL_VULKANL_H
#define PIXELCHAR_IMPL_VULKANL_H

#include <pixelchar/pixelchar.h>
#include <vulkan/vulkan.h>


#define PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION_LIST \
	PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION(vkGetDeviceProcAddr)\


#define PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION_LIST \
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDeviceWaitIdle)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkQueueWaitIdle)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkQueueSubmit)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkAllocateMemory)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkFreeMemory)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkBindBufferMemory)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkGetBufferMemoryRequirements)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkMapMemory)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkUnmapMemory)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkFlushMappedMemoryRanges)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateShaderModule)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyShaderModule)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateDescriptorSetLayout)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreatePipelineLayout)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyPipelineLayout)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateGraphicsPipelines)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyPipeline)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateCommandPool)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyCommandPool)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkAllocateCommandBuffers)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkFreeCommandBuffers)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkResetCommandBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkBeginCommandBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkEndCommandBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateDescriptorPool)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyDescriptorPool)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkAllocateDescriptorSets)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkUpdateDescriptorSets)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCreateFence)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkDestroyFence)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkResetFences)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkWaitForFences)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdCopyBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdPipelineBarrier)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdBindDescriptorSets)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdBindPipeline)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdBindVertexBuffers)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdBindIndexBuffer)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdPushConstants)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdDrawIndexed)\
	PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(vkCmdUpdateBuffer)

#define PIXELCHAR_IMPL_VULKAN_MAX_RENDERPASS_COUNT 8
#define PIXELCHAR_IMPL_VULKAN_MAX_SUBPASS_COUNT 8

typedef struct PixelcharImplVulkanInternalBufferAndMemory
{
	VkBuffer buffer;
	size_t bufferSize;
	VkMemoryRequirements memoryRequirements;

	VkDeviceMemory memory;
	size_t memorySize;
	VkMemoryPropertyFlags memoryPropertyFlags;
} PixelcharImplVulkanInternalBufferAndMemory;

typedef struct PixelcharImplVulkanFactory
{
	struct
	{
#define PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION(name) PFN_##name name;
		PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION_LIST
#undef PIXELCHAR_IMPL_VULKAN_INTERNAL_INSTANCE_FUNCTION

#define PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION(name) PFN_##name name;
		PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION_LIST
#undef PIXELCHAR_IMPL_VULKAN_INTERNAL_DEVICE_FUNCTION
	} func;

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkPhysicalDeviceProperties deviceProperties;
	uint32_t queueFamilyIndex;
	VkQueue queue;

	VkFence fence;
	VkCommandPool cmdPool;
	VkCommandBuffer cmd;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	PixelcharImplVulkanInternalBufferAndMemory index;
	PixelcharImplVulkanInternalBufferAndMemory staging;
	void* stagingBufferHostHandle;
} PixelcharImplVulkanFactory;

typedef struct PixelcharImplVulkanRenderer
{	
	PixelcharImplVulkanFactory* pFactory;

	VkPipeline pipeline;
} PixelcharImplVulkanRenderer;

typedef struct PixelcharImplVulkanFont
{
	PixelcharImplVulkanFactory* pFactory;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	PixelcharImplVulkanInternalBufferAndMemory bufferAndMemory;
} PixelcharImplVulkanFont;

VkResult PixelcharImplVulkanFactoryCreate(
	VkInstance instance,
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	uint32_t queueFamilyIndex,
	VkQueue queue,
	PFN_vkGetInstanceProcAddr pfnvkGetInstanceProcAddr,
	PixelcharImplVulkanFactory* pFactory,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplVulkanFactoryDestroy(PixelcharImplVulkanFactory* pFactory);

VkResult PixelcharImplVulkanRendererCreate(
	PixelcharImplVulkanFactory* pFactory,
	VkRenderPass renderPass,
	uint32_t subpass,
	const uint8_t* pCustomVertexShaderSource,
	size_t customVertexShaderSourceSize,
	const uint8_t* pCustomFragmentShaderSource,
	size_t customFragmentShaderSourceSize,
	PixelcharImplVulkanRenderer* pRenderer,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplVulkanRendererDestroy(PixelcharImplVulkanRenderer* pRenderer);

VkResult PixelcharImplVulkanFontCreate(
	PixelcharImplVulkanFactory* pFactory,
	PixelcharFont* pSourceFont,
	PixelcharImplVulkanFont* pFont,
	PixelcharLogCallback logCallback,
	void* logCallbackUserParam
);

void PixelcharImplVulkanFontDestroy(PixelcharImplVulkanFont* pFont);

void PixelcharImplVulkanRender(
	PixelcharImplVulkanRenderer* pRenderer,
	PixelcharImplVulkanFont* pFont,
	uint32_t characterCount,
	VkBuffer vertexBuffer,
	uint32_t vertexBufferOffset,
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
